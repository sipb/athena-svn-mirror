/* backend.c */

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
#if defined(MSDOS) || defined(SYS5) || defined(vaxc) || defined(_AUX_SOURCE)
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

static type_t *ulong_int_typep;
static type_t *ushort_int_typep;

void CSPELL_parameter_list __PROTOTYPE((FILE *fid, parameter_t *pp));
void CSPELL_func_decl __PROTOTYPE((FILE *fid, type_t *type, NAMETABLE_id_t name, parameter_t *paramlist));
void CSPELL_scalar_type_suffix __PROTOTYPE((FILE *fid, type_t *tp));

void exit __PROTOTYPE((int));
    
#ifdef __STDC__
void dename_interface (interface_t *ip);
#else
void dename_interface ();
#endif


void BACKEND_init()
{
    ulong_int_typep  = gen_type_node(long_unsigned_k);
    ushort_int_typep = gen_type_node(short_unsigned_k);
}

#ifdef __STDC__
void unimp_type_exit (type_kind kind, char *label)
#else
void unimp_type_exit (kind, label)
type_kind kind;
char      *label;
#endif
{
    extern boolean graceful_backend_exit;

    fprintf (stderr, "Unexpected (or unimplemented) type (");
    print_type_kind (stderr, kind);
    fprintf(stderr, ") encountered in %s", label);
    if (graceful_backend_exit) {
        fprintf(stderr, "; exiting.\n");
        exit(pgm_$error);
        }
    fprintf(stderr, ".\n");
}



#ifdef __STDC__
void CSPELL_cast_exp (FILE *fid, type_t *tp)
#else
void CSPELL_cast_exp (fid, tp)
FILE   *fid;
type_t *tp;
#endif
{
    fprintf (fid, "(");
    CSPELL_typed_name (fid, tp, NAMETABLE_NIL_ID);
    fprintf (fid, ")");
}


#ifdef __STDC__
void spell_name (FILE *fid, NAMETABLE_id_t name)
#else
void spell_name (fid, name)
FILE *fid;
NAMETABLE_id_t name;
#endif
{
    char *str;

    NAMETABLE_id_to_string (name, &str);
    fprintf (fid, "%s", str);
}


/* for use under dbx */
#ifndef __STDC__
void print_name (name)
NAMETABLE_id_t name;
#else
void print_name (NAMETABLE_id_t name)
#endif
{
    char *str;

    NAMETABLE_id_to_string (name, &str);
    printf ("name %d is \"%s\"\n", name, str);
}




static
#ifdef __STDC__
NAMETABLE_id_t NAMETABLE_add_numeric_exp (long i)
#else
NAMETABLE_id_t NAMETABLE_add_numeric_exp (i)
long i;
#endif
{
    char       exp[max_string_len];

    sprintf (exp, "%ld", i);
    return NAMETABLE_add_id (exp, false);
}



static
#ifdef __STDC__
boolean array_type (type_t *tp)
#else
boolean array_type (tp)
type_t *tp;
#endif
{
    switch (tp->kind) {
        case fixed_array_k:
        case open_array_k:
        case fixed_string_zero_k:
            return true;
        default:
            return false;
        }
    /*lint -unreachable */
}

static
#ifdef __STDC__
boolean open_type (type_t *tp)
#else
boolean open_type (tp)
type_t *tp;
#endif
{
    field_t *fp;

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
        case long_real_k:
        case hyper_integer_k:
        case hyper_unsigned_k:
        case fixed_array_k:
        case fixed_string_zero_k:
            return false;

        case open_array_k:
        case open_record_k:
        case open_string_zero_k:
            return true;

        case pointer_k:
            return open_type (tp->type_structure.pointer.pointee);

        case record_k:
            /* only non-variant records can be open */
            if (tp->type_structure.record.variant != NULL)
                return false;
            /* only the last field can be open so only check it */
            for (fp = tp->type_structure.record.fields;
                 fp->next_field;
                 fp = fp->next_field)
               ;
            return open_type(fp->type);

        case user_marshalled_k:
            return 
                open_type (tp->type_structure.user_marshalled.xmit_type);

        default:
            unimp_type_exit (tp->kind, "open_type");
            return 0;
        }
    /*lint -unreachable */
}


static
#ifdef __STDC__
boolean scalar_type (type_t *type)
#else
boolean scalar_type (type)
type_t *type;
#endif
{
    switch (type->kind) {
        case boolean_k:
        case byte_k:
        case character_k:
        case short_integer_k:
        case small_integer_k:
        case long_integer_k:
        case hyper_integer_k:
        case short_unsigned_k:
        case small_unsigned_k:
        case long_unsigned_k:
        case hyper_unsigned_k:
        case short_bitset_k:
        case small_bitset_k:
        case long_bitset_k:
        case short_enumeration_k:
        case small_enumeration_k:
        case long_enumeration_k:
        case short_real_k:
        case long_real_k:
        case short_subrange_k:
        case small_subrange_k:
        case long_subrange_k:
            return true;
        default:
            return false;
        }
    /*lint -unreachable */
}


static
#ifdef __STDC__
binding_t *select_routine_bindings (binding_t *bp)
#else
binding_t *select_routine_bindings (bp)
binding_t *bp;
#endif
{
    binding_t *rl_head = NULL, *rl_tail; /* routine list head, tail */
    int       op_number = 0;

    for (; bp; bp = bp->next)
        if (bp->kind == routine_k) {
            bp->binding->routine.op_number = op_number;
            op_number++;
            if (rl_head == NULL)
                rl_head = rl_tail = bp;
            else {
                rl_tail->next = bp;
                rl_tail = bp;
                }
            }

    if (rl_head != NULL)
        rl_tail->next = NULL;

    return rl_head;
}


typedef enum {
    client_switch,
    client_stub,
    server_stub
    } output_kind;

static
#ifdef __STDC__
void CSPELL_std_include (FILE *fid, char name[], output_kind thingy)
#else
void CSPELL_std_include (fid, name, thingy)
FILE *fid;
char name[];
output_kind thingy;
#endif
{
    fprintf (fid, "#define NIDL_GENERATED\n#define NIDL_");
    switch (thingy) {
        case client_switch:
            fprintf (fid, "CSWTCH");
            break;
        case client_stub:
            fprintf (fid, "CSTUB");
            break;
        case server_stub:
            fprintf (fid, "SSTUB");
        }
    fprintf (fid, "\n#include \"%s.h\"\n", name);
}


static
#ifdef __STDC__
void CSPELL_proto_and_synopsis (FILE *fid, parameter_t *pp)
#else
void CSPELL_proto_and_synopsis (fid, pp)
FILE           *fid;
parameter_t    *pp;
#endif
{
    boolean            first = true;
    parameter_t        *saved_pp;

    fprintf (fid, "\n#ifdef __STDC__\n");

        CSPELL_parameter_list (fid, pp);
        fprintf (fid, "\n");

    fprintf (fid, "#else\n");

        fprintf (fid, " (");
        saved_pp = pp;
        for (; pp; pp = pp->next_param) {
            if (first)
                first = false;
            else
                fprintf (fid, ", ");
            spell_name (fid, pp->name);
            }
        fprintf (fid, ")\n");

        for (pp = saved_pp; pp; pp = pp->next_param)
            CSPELL_var_decl (fid, pp->type, pp->name);

    fprintf (fid, "#endif\n");
}


static
#ifdef __STDC__
void gen_client_switch (FILE *fid, NAMETABLE_id_t if_name, char leaf_name[],
    binding_t *rbp, boolean f77_client)
#else
void gen_client_switch (fid, if_name, leaf_name, rbp, f77_client)
FILE           *fid;
NAMETABLE_id_t if_name;
char           leaf_name[];
binding_t      *rbp; /* routine bindings pointer */
boolean        f77_client;
#endif
{
    parameter_t        *pp;
    routine_t          *rp;
    boolean            first;
    NAMETABLE_id_t     routine_name;

    CSPELL_std_include (fid, leaf_name, client_switch);

    for (; rbp; rbp = rbp->next) {

        rp = &rbp->binding->routine;
        pointerize_routine (rp);

        /* append a "_" to routine name if it's going to called from unix fortran code */
        routine_name = f77_client ? NAMETABLE_add_derived_name (rbp->name, "%s_") : rbp->name;

        /* emit routine type, declarator, and parameter list */
        fprintf (fid, "\n");
        CSPELL_typed_name (fid, rp->result->type, routine_name);

        /* emit a prototype and a synopsis style header for the switch routine */
        CSPELL_proto_and_synopsis (fid, rp->parameters);

        /* emit body */
        fprintf (fid, "{\n");

        if (rp->result && rp->result->type->kind != void_k)
            fprintf (fid, "return ");
        fprintf (fid, "(*");
        spell_name (fid, NAMETABLE_add_derived_name (if_name, "%s$client_epv"));
        fprintf (fid, ".");
        spell_name (fid, rbp->name);
        fprintf (fid, ")(");
        first = true;
        for (pp = rp->parameters; pp; pp = pp->next_param) {
            if (first)
                first = false;
            else
                fprintf (fid, ", ");
            spell_name (fid, pp->name);
            }
        fprintf (fid, ");\n");

        fprintf (fid, "}\n");
        }
}


static
#ifdef __STDC__
void CSPELL_csr_header (FILE *fid, NAMETABLE_id_t name, parameter_t *pp, parameter_t *resultp)
#else
void CSPELL_csr_header (fid, name, pp, resultp)
FILE           *fid;
NAMETABLE_id_t name;
parameter_t    *pp;
parameter_t    *resultp;
#endif
{
    boolean            pointer;
    array_dimension_t  *array_dimensions;
    boolean            function;
    parameter_t        *paramlist;
    boolean            pointer_elements;

    fprintf (fid, "\nstatic ");
    if (resultp) {
        CSPELL_type_exp (fid, resultp->type, NAMETABLE_NIL_ID, &pointer, &array_dimensions, &function, &paramlist, &pointer_elements);
        fprintf (fid, " ");
        }
    else {
        fprintf (fid, "void ");
        pointer = function = 0;
        array_dimensions = NULL;
        paramlist = NULL;
        }

    CSPELL_name_declarator (fid, NAMETABLE_add_derived_name (name, "%s_csr"), pointer,
                           array_dimensions, function, paramlist, pointer_elements);

    CSPELL_proto_and_synopsis (fid, pp);
}


static
#ifdef __STDC__
void CSPELL_local_var_decls (FILE *fid, routine_t *rp, side_t side)
#else
void CSPELL_local_var_decls (fid, rp, side)
FILE      *fid;
routine_t *rp;
side_t    side;
#endif
{
    local_var_t        *vp;
    boolean            first = true;

    for (vp = rp->local_vars; vp; vp = vp->next_local) {
        if (vp->side != both_sides && vp->side != side)
            continue;

        if (first) {
            first = false;
            fprintf (fid, "\n/* local variables */\n");
            }

        if (vp->volatility)
            fprintf (fid, "volatile ");
        CSPELL_typed_name (fid, vp->type, vp->name);
        if (vp->comment) {
            fprintf (fid, " /* ");
            spell_name (fid, vp->comment);
            fprintf (fid, " */ ");
            }
        fprintf (fid, ";\n");
        }
}

#if 0
/* Not referenced */

static
#ifdef __STDC__
void CSPELL_marshalling_init (FILE *fid, NAMETABLE_id_t binding_name)
#else
void CSPELL_marshalling_init (fid, binding_name)
FILE *fid;
NAMETABLE_id_t binding_name;
#endif
{
    fprintf (fid, "\n/* marshalling init */\n");
    fprintf (fid, "data_offset=");
    spell_name (fid, binding_name);
    fprintf (fid, "->data_offset;\n");
}
#endif

static
#ifdef __STDC__
short required_alignment (type_t *tp, boolean last_is_ref)
#else
short required_alignment (tp, last_is_ref)
type_t *tp;
boolean last_is_ref;
#endif
{
    /*
         returns alignment required for the type
    */

    field_t *fp;

    switch (tp->kind) {

        case boolean_k:
        case byte_k:
        case character_k:
        case small_bitset_k:
        case small_enumeration_k:
        case small_integer_k:
        case small_unsigned_k:
        case small_subrange_k:
            return 1;

        case short_bitset_k:
        case short_integer_k:
        case short_unsigned_k:
        case short_subrange_k:
        case short_enumeration_k:
            return 2;

        case short_real_k:
        case long_enumeration_k:
        case long_bitset_k:
        case long_integer_k:
        case long_unsigned_k:
        case long_subrange_k:
            return 4;

        case hyper_integer_k:
        case hyper_unsigned_k:
        case long_real_k:
            return 8;

        case pointer_k:
            return
                required_alignment (
                    tp->type_structure.pointer.pointee,
                    false);

        case user_marshalled_k:
            return
                required_alignment (
                    tp->type_structure.user_marshalled.xmit_type,
                    false);

        case record_k:
            if ((fp = tp->type_structure.record.fields) != NULL)
                return required_alignment (
                    fp->type,
                    (boolean) (fp->last_is_ref != NULL));
            else /* it has no fields; it should have a variant part */
                return required_alignment (
                    tp->type_structure.record.variant->tag_type,
                    false);

        case open_record_k:
        case open_array_k:
            return 2;   /* for short integer representing max allocation */

        case fixed_array_k:
            return
                last_is_ref ? 2 
                            : required_alignment (
                                  tp->type_structure.fixed_array.elements,
                                  false);

        case fixed_string_zero_k:
        case open_string_zero_k:
            return 2;    /* for the short integer prepended to the string rep in the packet */

        default:
            unimp_type_exit (tp->kind, "required_alignment");
            return 0;
        }
    /*lint -unreachable */
}


static
#ifdef __STDC__
short resulting_alignment (type_t *tp, boolean last_is_ref);
#else
short resulting_alignment ();
#endif

static
#ifdef __STDC__
short resulting_alignment_field_list (field_t *fp)
#else
short resulting_alignment_field_list (fp)
field_t *fp;
#endif
{
    for (; fp->next_field; fp = fp->next_field)
        ;
    return resulting_alignment (fp->type, fp->last_is_ref != NULL);
}


static
#ifdef __STDC__
short resulting_alignment (type_t *tp, boolean last_is_ref)
#else
short resulting_alignment (tp, last_is_ref)
type_t *tp;
boolean last_is_ref;
#endif
{
    component_t *cp;
    short answer, temp_answer;
    variant_t *vp;

    extern boolean support_bug[];

    switch (tp->kind) {

        case boolean_k:
        case byte_k:
        case character_k:
        case small_bitset_k:
        case small_enumeration_k:
        case small_integer_k:
        case small_unsigned_k:
        case small_subrange_k:
            return 1;

        case short_bitset_k:
        case short_integer_k:
        case short_unsigned_k:
        case short_subrange_k:
        case short_enumeration_k:
            return 2;

        case short_real_k:
        case long_enumeration_k:
        case long_bitset_k:
        case long_integer_k:
        case long_unsigned_k:
        case long_subrange_k:
            return 4;

        case hyper_integer_k:
        case hyper_unsigned_k:
        case long_real_k:
            return 8;

        case pointer_k:
            return
                resulting_alignment (
                    tp->type_structure.pointer.pointee,
                    false);

        case user_marshalled_k:
            return
                resulting_alignment (
                    tp->type_structure.user_marshalled.xmit_type,
                    false);

        case record_k:
        case open_record_k:
            if ((vp = tp->type_structure.record.variant) != NULL) {
                answer = rpc_$max_alignment;
                for (cp = vp->components; cp; cp = cp->next_component)
                    if ((temp_answer = resulting_alignment_field_list (cp->fields)) < answer)
                        answer = temp_answer;
                return answer;
                }
            else
                /* if has no variant part it better have a fixed part */
                return resulting_alignment_field_list (tp->type_structure.record.fields);

        case open_array_k:
            answer = resulting_alignment (
                tp->type_structure.open_array.elements,
                false);
            return (!support_bug[1] && (answer > 2)) ? 2 : answer;

        case fixed_array_k:
            answer = resulting_alignment (
                tp->type_structure.fixed_array.elements,
                false);
            return (!support_bug[1] && last_is_ref && (answer > 2)) ? 2 : answer;

        case fixed_string_zero_k:
        case open_string_zero_k:
            return 1;

        default:
            unimp_type_exit (tp->kind, "resulting_alignment");
            return 0;
        }
        /*lint -unreachable */
}


static
#ifdef __STDC__
long fixed_size_type (type_t *tp, boolean last_is_ref);
#else
long fixed_size_type ();
#endif

static
#ifdef __STDC__
long fixed_size_field_list (field_t *fp)
#else
long fixed_size_field_list (fp)
field_t     *fp;
#endif
{
    long total, subtotal, breakage;
    boolean first;
    short alignment, prior_alignment;

    total = 0;
    breakage = 0;
    first = true;
    prior_alignment = 0;        /* avoid warning from simple-minded lint */
    
    for (; fp; fp = fp->next_field) {

        if (first)
            first = false;
        else {
            alignment = required_alignment (fp->type, (boolean) (fp->last_is_ref != NULL));
            if (alignment > prior_alignment)
                breakage += alignment - prior_alignment;
            }
        prior_alignment = resulting_alignment (fp->type, (fp->last_is_ref != NULL));

        subtotal = fixed_size_type (fp->type, (boolean) (fp->last_is_ref != NULL));
        if (!subtotal)
            return 0;
        total += subtotal;
        }
    return total + breakage;
}

static
#ifdef __STDC__
long fixed_size_type (type_t *tp, boolean last_is_ref)
#else
long fixed_size_type (tp, last_is_ref)
type_t      *tp;
boolean     last_is_ref;
#endif
{
    /*
         returns size of the type if it is fixed at compile time
         and returns 0 if not

         (thus, can be used as a predicate and as a metric)
    */

    component_t   *cp;
    long          component_alignment;
    long          effective_size;
    long          element_size;
    type_t        *element_tp;
    long          fields_size;
    field_t       *fp;
    long          gap_size;
    array_index_t *indexp;
    long          inter_element_breakage;
    long          max_effective_size;
    long          prior_align;
    long          size;
    long          tag_size;
    type_t        *tag_tp;
    variant_t     *vp;
    

    switch (tp->kind) {

        case drep_k:
        case handle_k:
            return true; /* should not be used as the size of a handle_t or rpc_$drep_t */

        case boolean_k:
        case byte_k:
        case character_k:
        case small_bitset_k:
        case small_enumeration_k:
        case small_integer_k:
        case small_unsigned_k:
        case small_subrange_k:
            return 1;

        case short_bitset_k:
        case short_integer_k:
        case short_unsigned_k:
        case short_subrange_k:
        case short_enumeration_k:
            return 2;

        case short_real_k:
        case long_enumeration_k:
        case long_bitset_k:
        case long_integer_k:
        case long_unsigned_k:
        case long_subrange_k:
            return 4;

        case hyper_integer_k:
        case hyper_unsigned_k:
        case long_real_k:
            return 8;

        case pointer_k:
            return fixed_size_type (tp->type_structure.pointer.pointee, last_is_ref);

        case user_marshalled_k:
            return fixed_size_type (tp->type_structure.user_marshalled.xmit_type, false);

        case fixed_array_k:
            if (last_is_ref)
                return 0;
            element_tp = tp->type_structure.fixed_array.elements;
            if (!(element_size = fixed_size_type (element_tp, false)))
                return 0;
            size = element_size;
            inter_element_breakage =
                required_alignment (element_tp, false) -
                resulting_alignment (element_tp, false);
            if (inter_element_breakage > 0)
                size += inter_element_breakage;
            for (indexp = tp->type_structure.fixed_array.indices; indexp; indexp = indexp->next)
                size = size * cardinality (indexp->type->type_structure.subrange);
            return size;

        case fixed_string_zero_k:
        case open_string_zero_k:
            return 0; /* size is "strlen()" */

        case open_array_k:
        case open_record_k:
            return 0;

        case record_k:
            fp = tp->type_structure.record.fields;
            if (fp != NULL) {
                fields_size = fixed_size_field_list (fp);
                if (fields_size == 0)
                    return 0;
                    }
            else
                fields_size = 0;

            /* if here then the non variant part is of fixed size == fields_size 
               and there may be a variant part */
            vp = tp->type_structure.record.variant;
            if (vp == NULL) 
                return fields_size;
            else {
                /* the variant part of records is considered to be fixed
                   size: no variants containing open arrays are permitted
                   by the frontend and the worst case is assumed for
                   allocation requirements so that the compiler does not
                   need to emit switch statements to determine the size
                  of the actual variant at run time */

                tag_tp = vp->tag_type;

                if (fp != NULL) {
                    gap_size =
                        required_alignment (tag_tp, false) -
                        resulting_alignment_field_list (fp);
                    if (gap_size < 0)
                        gap_size = 0;
                    }
                else
                    gap_size = 0;

                tag_size = fixed_size_type (tag_tp, false);

                prior_align = resulting_alignment (tag_tp, false);

                max_effective_size = 0;
                for (cp = vp->components; cp; cp = cp->next_component) {
                    effective_size = fixed_size_field_list (cp->fields);
                    component_alignment =
                        (cp->fields == NULL) ?
                        1 :
                        required_alignment (cp->fields->type, false);
                    if (prior_align < component_alignment)
                        effective_size += component_alignment - prior_align;
                    if (max_effective_size < effective_size)
                        max_effective_size = effective_size;
                    }

                return fields_size + gap_size + tag_size + max_effective_size;
                }

        default:
            unimp_type_exit (tp->kind, "fixed_size_type");
            return 0;
        }
    /*lint -unreachable */
}

static
#ifdef __STDC__
type_t *remove_named_type_nodes (type_t *tp)
#else
type_t *remove_named_type_nodes (tp)
type_t *tp;
#endif
{
    type_t *real_type_node, *new_type_node;

    component_t *cp;
    field_t *fp;
    parameter_t *pp;
    structure_t *ntsp;
    variant_t *vp;

    if (tp == NULL)
        return NULL;

    switch (tp->kind) {

        case named_k:
            /* find the real type node behind this 'named type' node */
            real_type_node = remove_named_type_nodes(tp->type_structure.named.resolution);

            /* if the real type node has already been marked with another name replicate it */
            if (real_type_node->type_name != NAMETABLE_NIL_ID
            &&  real_type_node->type_name != tp->type_structure.named.name) {
                ntsp = AST_type_node (void_k);
                new_type_node = &ntsp->type;
                *new_type_node = *real_type_node;
                real_type_node = new_type_node;
                }

            /* label the real type node with its name */
            real_type_node->type_name = tp->type_structure.named.name;

            /* can't deallocate the 'named type' node
               because some other node may still point to it */
            return real_type_node;

        case boolean_k:
        case byte_k:
        case short_enumeration_k:
        case long_enumeration_k:
        case small_enumeration_k:
        case small_unsigned_k:
        case small_integer_k:
        case short_integer_k:
        case short_subrange_k:
        case short_unsigned_k:
        case long_integer_k:
        case long_subrange_k:
        case hyper_unsigned_k:
        case hyper_integer_k:
        case long_unsigned_k:
        case character_k:
        case short_real_k:
        case long_real_k:
        case fixed_string_zero_k:
        case open_string_zero_k:
        case drep_k:
        case handle_k:
        case void_k:
            return tp;

        case fixed_array_k:
            tp->type_structure.fixed_array.elements =
                remove_named_type_nodes (tp->type_structure.fixed_array.elements);
            return tp;

        case open_array_k:
            tp->type_structure.open_array.elements =
                remove_named_type_nodes (tp->type_structure.open_array.elements);
            return tp;

        case small_bitset_k:
        case short_bitset_k:
        case long_bitset_k:
            tp->type_structure.bitset.base_type =
                remove_named_type_nodes (tp->type_structure.bitset.base_type);
            return tp;

        case pointer_k:
            tp->type_structure.pointer.pointee =
                remove_named_type_nodes(tp->type_structure.pointer.pointee);
            return tp;

        case record_k:
            for ( fp = tp->type_structure.record.fields; fp; fp = fp->next_field)
                fp->type =
                    remove_named_type_nodes (fp->type);
            if ((vp = tp->type_structure.record.variant) != NULL) {
                vp->tag_type =
                    remove_named_type_nodes (vp->tag_type);
                for (cp = vp->components; cp; cp = cp->next_component)
                    for ( fp = cp->fields; fp; fp = fp->next_field)
                        fp->type =
                            remove_named_type_nodes (fp->type);
                }
            return tp;

        case routine_ptr_k:
            for (pp = tp->type_structure.routine_ptr.parameters; pp; pp = pp->next_param)
                pp->type = remove_named_type_nodes (pp->type);
            tp->type_structure.routine_ptr.result->type =
                remove_named_type_nodes(tp->type_structure.routine_ptr.result->type);
            return tp;

        case user_marshalled_k:
            tp->type_structure.user_marshalled.xmit_type =
                remove_named_type_nodes(tp->type_structure.user_marshalled.xmit_type);
            return tp;

        default:
            unimp_type_exit (tp->kind, "remove_named_type_nodes");
        }
    /*lint -unreachable */
}


static void
#ifdef __STDC__
dename_parameter (parameter_t *pp)
#else
dename_parameter (pp)
parameter_t *pp;
#endif
{
    pp->type = remove_named_type_nodes (pp->type);
}


static void
#ifdef __STDC__
dename_routine (routine_t *rp)
#else
dename_routine (rp)
routine_t *rp;
#endif
{
    parameter_t *pp;

    for (pp = rp->parameters; pp; pp = pp->next_param)
        dename_parameter (pp);
    dename_parameter (rp->result);
}


static void
#ifdef __STDC__
dename_constant (constant_t *cp)
#else
dename_constant (cp)
constant_t *cp;
#endif
{
    if (cp->kind == enum_k)
        cp->value.enum_val.base_type =
            remove_named_type_nodes (cp->value.enum_val.base_type);
}


static void
#ifdef __STDC__
dename_binding (binding_t *bp)
#else
dename_binding (bp)
binding_t *bp;
#endif
{
    switch (bp->kind) {
        case constant_k:
            dename_constant (&bp->binding->constant);
            break;

        case routine_k:
            dename_routine (&bp->binding->routine);
            break;

        case type_k:
            bp->binding =
                (structure_t *) remove_named_type_nodes (&bp->binding->type);
        }
}


static void
#ifdef __STDC__
dename_interface (interface_t *ip)
#else
dename_interface (ip)
interface_t *ip;
#endif
{
    binding_t   *bp;

    ip->implicit_handle_type = remove_named_type_nodes(ip->implicit_handle_type);
    for (bp = ip->exports; bp; bp = bp->next)
        dename_binding (bp);
}


static
#ifdef __STDC__
NAMETABLE_id_t open_delta_exp (type_t *tp, helpers_t *hp)
#else
NAMETABLE_id_t open_delta_exp (tp, hp)
type_t *tp;
helpers_t *hp;
#endif
{
    field_t  *fp;
    helpers_t *fhp;

    if (tp == NULL)
        return NAMETABLE_NIL_ID;

    switch (tp->kind) {

        case pointer_k:
            return open_delta_exp (
                tp->type_structure.pointer.pointee,
                hp);

/*
        This case can't happen until embedded user_marshalled types are
        allowed; whether to call (recursively) open_delta_exp with the
        marshall_helpers or with unmarshall_helpers is dependent on whether
        we're trying to construct the helpers structure for a higher level
        user_marshalled type's helpers_t's marshall_helpers or
        unmarshall_helpers field.  (This is more evidence that the right
        treatment of embedded user_marshalled types entails making the
        including type be a user_marshalled_k with a derived [transmit_as()]
        type.)

        In other words we need to pass more state through the interfaces
        of the "patch_" family of routines to indicate whether a
        marshalling_helper or an unmarshalling_helper is being constructed.

        But we punt the issue for now since only top level user_marshalled_k
        type nodes will occur in version 1.0

        case user_marshalled_k:
            return open_delta_exp (
                tp->type_structure.user_marshalled.xmit_type,
                (building_marshall_helpers)
                ? hp->helpers.user_marshalled_h.marshall_helpers
                : hp->helpers.user_marshalled_h.unmarshall_helpers
                );
*/
        case open_array_k:
            return hp->helpers.array_h.ss_alloc_exp;

        case open_string_zero_k:
            return hp->helpers.string_h.alloc_exp;

        case open_record_k:
            /* go to end of list of fields then look at last field */
            fhp = hp->helpers.record_h.fields;
            for (fp = tp->type_structure.record.fields; fp->next_field; fp = fp->next_field)
                fhp = fhp->next;
            return open_delta_exp (fp->type, fhp);
            /* NIDL checker assures that variants do not have open types in them */

        default:
            unimp_type_exit (tp->kind, "open_delta_exp");
            return 0;

        }
    /*lint -unreachable*/
}


static
#ifdef __STDC__
type_t *open_base_type (type_t *tp)
#else
type_t *open_base_type (tp)
type_t *tp;
#endif
{
    field_t     *fp;

    if (tp == NULL)
        return tp;

    switch (tp->kind) {

        case open_array_k:
            return tp->type_structure.open_array.elements;

        case open_string_zero_k:
            return gen_type_node(character_k);

        case pointer_k:
            return open_base_type (tp->type_structure.pointer.pointee);

        case user_marshalled_k:
            return open_base_type (tp->type_structure.user_marshalled.xmit_type);

        case open_record_k:
            /* go to end of list of fields then look at last field */
            for (fp = tp->type_structure.record.fields; fp->next_field; fp = fp->next_field)
               ;
            return open_base_type (fp->type);
            /* NIDL checker assures that variants do not have open types in them */

        default:
            unimp_type_exit (tp->kind, "open_base_type");
            return 0;

        }
    /*lint -unreachable */
}


static
#ifdef __STDC__
void declare_local_var (NAMETABLE_id_t name, type_t *type, side_t side, routine_t *rp, NAMETABLE_id_t comment, boolean volatility)
#else
void declare_local_var (name, type, side, rp, comment, volatility)
NAMETABLE_id_t name;
type_t         *type;
side_t         side;
routine_t      *rp;
NAMETABLE_id_t comment;
boolean        volatility;
#endif
{
    local_var_t *vp;

    vp = (local_var_t *) alloc (sizeof(local_var_t));
    vp->name = name;
    vp->type = type;
    vp->side = side;
    vp->next_local = rp->local_vars;
    vp->comment = comment;
    vp->volatility = volatility;
    rp->local_vars = vp;
   
}


static
#ifdef __STDC__
NAMETABLE_id_t declare_short_local_var (NAMETABLE_id_t name, type_t *type, side_t side, routine_t *rp, boolean volatility)
#else
NAMETABLE_id_t declare_short_local_var (name, type, side, rp, volatility)
NAMETABLE_id_t name;
type_t         *type;
side_t         side;
routine_t      *rp;
boolean        volatility;
#endif
{
    NAMETABLE_id_t short_name;
    char           str [sizeof ("xXx_ffffffff")];

    sprintf (str, "xXx_%x_", name);
    short_name = NAMETABLE_add_id (str, false);
    declare_local_var (short_name, type, side, rp, name, volatility);
    return short_name;
}


static
#ifdef __STDC__
helpers_t *alloc_helper_node (NAMETABLE_id_t cs_mexp, NAMETABLE_id_t ss_mexp)
#else
helpers_t *alloc_helper_node (cs_mexp, ss_mexp)
NAMETABLE_id_t cs_mexp;
NAMETABLE_id_t ss_mexp;
#endif
{
    helpers_t *hp;

    hp = (helpers_t *) alloc (sizeof (helpers_t));

    hp->cs_mexp = cs_mexp;
    hp->ss_mexp = ss_mexp;

    return hp;
}


static
#ifdef __STDC__
type_t *pointer_type (type_t *tp)
#else
type_t *pointer_type (tp)
type_t *tp;
#endif
{
    /*
        given a *type_t which denotes type "a" pointer_type returns a pointer
        to a type node which denotes the type "pointer to a"
    */
    type_t *pnp;

    pnp = gen_type_node (pointer_k);
    pnp->type_structure.pointer.pointee = tp;

    return pnp;
}

static
#ifdef __STDC__
helpers_t *patch_node (
    char     *last_is_ref,
    char     *max_is_ref,
    NAMETABLE_id_t name,
    NAMETABLE_id_t cs_mexp,
    NAMETABLE_id_t ss_mexp,
    type_t         *type,
    boolean        parameter_flag,
    parameter_t    *pp,
    routine_t      *rp);
#else
helpers_t *patch_node ();
#endif


static
#ifdef __STDC__
void patch_field_node (
    NAMETABLE_id_t name,
    NAMETABLE_id_t cs_mexp,
    NAMETABLE_id_t ss_mexp,
    field_t *fp,
    parameter_t *pp,
routine_t *rp)
#else
void patch_field_node (name, cs_mexp, ss_mexp, fp, pp, rp)
NAMETABLE_id_t name;
NAMETABLE_id_t cs_mexp;
NAMETABLE_id_t ss_mexp;
field_t        *fp;
parameter_t    *pp;
routine_t      *rp;
#endif
{

    /* done if field already patched (by recursive call on another field's
         last_is_ref or max_is_ref */
    if (fp->temp_helpers_p != NULL)
        return;

    /* patch this field's last_ref_ref and max_is_ref before patching it */
    if (fp->last_is_ref != NULL)
        patch_field_node (name, cs_mexp, ss_mexp, fp->last_is_ref, pp, rp);
    if (fp->max_is_ref != NULL)
        patch_field_node (name, cs_mexp, ss_mexp, fp->max_is_ref, pp, rp);

    /* patch this field i.e. store pointer to its helpers_t in a temporary
         pointer in the field_t */
    fp->temp_helpers_p =
        patch_node (
            (char *) fp->last_is_ref,
            (char *) fp->max_is_ref,
            NAMETABLE_add_derived_name2 (name,    fp->name, "%s%s_"),
            NAMETABLE_add_derived_name2 (cs_mexp, fp->name, "%s.%s"),
            NAMETABLE_add_derived_name2 (ss_mexp, fp->name, "%s.%s"),
            fp->type,
            false,          /* a field node, NOT a parameter node */
            pp,
            rp);
}


static
#ifdef __STDC__
helpers_t *patch_field_list (
    NAMETABLE_id_t name,
    NAMETABLE_id_t cs_mexp,
    NAMETABLE_id_t ss_mexp,
    field_t        *field_list,
    parameter_t    *pp,
    routine_t      *rp)
#else
helpers_t *patch_field_list (name, cs_mexp, ss_mexp, field_list, pp, rp)
NAMETABLE_id_t name;
NAMETABLE_id_t cs_mexp;
NAMETABLE_id_t ss_mexp;
field_t        *field_list;
parameter_t    *pp;
routine_t      *rp;
#endif
{
    helpers_t *result;
    field_t *fp;

    /* call the recursive routine patch_field_node to set fp->temp_helpers_p
         for each in list element */
    for (fp = field_list; fp; fp = fp->next_field)
        patch_field_node (name, cs_mexp, ss_mexp, fp, pp, rp);

    /* link together helper structures and break links from field_t's to helpers_t's */
    if (field_list != NULL) {
        result = field_list->temp_helpers_p;
        for (fp = field_list; fp->next_field; fp = fp->next_field) {
            fp->temp_helpers_p->next = fp->next_field->temp_helpers_p;
            fp->temp_helpers_p = NULL;
            }
        fp->temp_helpers_p = NULL;
        return result;
        }
    else
        return NULL;
}


static
#ifdef __STDC__

helpers_t *patch_components (
    parameter_t    *last_is_ref,
    parameter_t    *max_is_ref,
    NAMETABLE_id_t name,
    NAMETABLE_id_t cs_mexp,
    NAMETABLE_id_t ss_mexp,
    component_t    *cp,
    parameter_t    *pp,
    routine_t      *rp)
#else
helpers_t *patch_components (last_is_ref, max_is_ref, name, cs_mexp, ss_mexp, cp, pp, rp)
parameter_t    *last_is_ref;
parameter_t    *max_is_ref;
NAMETABLE_id_t name;
NAMETABLE_id_t cs_mexp;
NAMETABLE_id_t ss_mexp;
component_t    *cp;
parameter_t    *pp;
routine_t      *rp;
#endif
{
    helpers_t *hp;
    field_t   *fp;
    NAMETABLE_id_t new_name;
    NAMETABLE_id_t new_cs_mexp;
    NAMETABLE_id_t new_ss_mexp;

    if (cp) {
        fp = cp->fields;
        if (fp != NULL && fp->next_field != NULL) {
            /* if there are multiple fields, then a struct named by cp->label was
               introduced in CSPELL_type_exp and it needs to accounted for in naming
               the value */
            new_name =    NAMETABLE_add_derived_name2 (name, cp->label, "%s%s_");
            new_cs_mexp = NAMETABLE_add_derived_name2 (cs_mexp, cp->label, "%s.%s");
            new_ss_mexp = NAMETABLE_add_derived_name2 (ss_mexp, cp->label, "%s.%s");
            }
        else {
            new_name =    name;
            new_cs_mexp = cs_mexp;
            new_ss_mexp = ss_mexp;
            }
        hp = alloc_helper_node (cs_mexp, ss_mexp);
        hp->helpers.component_h.fields =
            patch_field_list (new_name, new_cs_mexp, new_ss_mexp, cp->fields, pp, rp);
        hp->next =
            patch_components (
                last_is_ref, max_is_ref, name, cs_mexp, ss_mexp, cp->next_component, pp, rp);
        return hp;
        }
    else
        return NULL;
}


static
#ifdef __STDC__
helpers_t *patch_array_node (
    char           *last_is_ref,
    char           *max_is_ref,
    NAMETABLE_id_t name,
    NAMETABLE_id_t cs_mexp,
    NAMETABLE_id_t ss_mexp,
    array_index_t  *indices,
    type_t         **element_type_p,
    type_kind      array_kind,
    boolean        parameter_node,
    parameter_t    *pp,
    routine_t      *rp)
#else
helpers_t *patch_array_node (
    last_is_ref,
    max_is_ref,
    name,            /* prefix of helper variable names */
    cs_mexp,         /* client side marhalling expression */
    ss_mexp,         /* server side marhalling expression */
    indices,
    element_type_p,
    array_kind,
    parameter_node,  /* true => array is a parameter => (last,max)_is_ref are *parameter_t's
                        false => array is a field => (last,max)_is_ref are *field_t's */
    pp,
    rp)

char           *last_is_ref;
char           *max_is_ref;
NAMETABLE_id_t name;
NAMETABLE_id_t cs_mexp;
NAMETABLE_id_t ss_mexp;
array_index_t  *indices;
type_t         **element_type_p;
type_kind      array_kind;
boolean        parameter_node;
parameter_t    *pp;
routine_t      *rp;
#endif
{
    helpers_t      *hp;
    type_t         *pnp;
    NAMETABLE_id_t cs_limit_id;
    NAMETABLE_id_t ss_limit_id;
    NAMETABLE_id_t cs_alloc_exp_id;
    NAMETABLE_id_t ss_alloc_exp_id;
    NAMETABLE_id_t multiplier_id;
    long           multiplier;
    array_index_t  *indexp;
    char           index_string[sizeof("%s") + (MAX_ARRAY_INDICES * sizeof("[0]")) + 1];

    char           *field_name_strp;
    char           *mexp_strp;
    char           work_str[max_string_len];

    NAMETABLE_id_t last_is_cs_exp;
    NAMETABLE_id_t last_is_ss_exp;
    NAMETABLE_id_t max_is_cs_exp;
    NAMETABLE_id_t max_is_ss_exp;

    long           i;
    type_t         *base_type_p;

    /* remember the declared element type so that it can be used to cast
       the server side stand-in in the manager routine call */
    base_type_p = *element_type_p;

    /* if the array elements are arrays then convert to one multidimensioned */
    /* array, i.e. array[2][3] of array[4] of int ==> array[2][3][4] of int  */

    if (base_type_p->kind == fixed_array_k) {
        indices = AST_concat_indices (
                                indices,
                                base_type_p->type_structure.fixed_array.indices
                                );
        *element_type_p = base_type_p->type_structure.fixed_array.elements;
        hp = patch_array_node (
                 last_is_ref,
                 max_is_ref,
                 name,
                 cs_mexp,
                 ss_mexp,
                 indices,
                 element_type_p,
                 array_kind,
                 parameter_node,
                 pp,
                 rp);
        hp->helpers.array_h.declared_element_type = base_type_p;
        return hp;
        }

    /* allocate the helper structure to describe this array parameter */
    hp = alloc_helper_node (cs_mexp, ss_mexp);
    hp->helpers.array_h.declared_element_type = base_type_p;

    /* create a type node that denotes a pointer to the element type for
       use in typing helper variables */
    pnp = pointer_type (base_type_p);

    /* remember whether array elements are scalars */
    hp->helpers.array_h.scalar_elements = scalar_type (base_type_p);

    /* In some circumstances the ins packet on the server side can be used
       as the server side surrogate for the array.  This applies to the
       following cases:

           The symbol ALIGNED_SCALAR_ARRAYS must be defined in the
           compilation environment of the server side stub; this means
           that the naturally aligned rep of scalars in the packet is
           compatible with the code generated for the server side stub.
           This is checked at stub compile time because the NIDL compiler
           emit array marshalling code that is conditionally compiled
           based on ALIGNED_SCALAR_ARRAYS.

           Array elements must be scalars.

           Only IN or IN OUT arrays can be treated this way since only
           they have a rep in the ins packet.

           For IN-only arrays both fixed and varying sized arrays can
           be handled using the rep in the ins packet.

           For IN OUT arrays only fixed size arrays or arrays whose last_is
           attribute in IN only can be handled in this way since,
           otherwise, the out array rep might be larger than the in array
           rep.

       The following code tests for these cases and, if they are met,
       a flag is set in the helper structure and a variable for pointing
       at the arrays server side rep in the ins packet is declared and
       recorded in the helper structure.
    */

    if (
        hp->helpers.array_h.use_ins_packet =
            (   parameter_node
            &&  hp->helpers.array_h.scalar_elements
            &&  pp->in
            &&  (   !pp->out
                ||  last_is_ref == NULL
                ||  !((parameter_t *)last_is_ref)->out
                )
            )
       ) hp->helpers.array_h.ss_arg_exp =
            declare_short_local_var (
                NAMETABLE_add_derived_name (name, "%sipp_"), /* ins packet pointer */
                pnp,
                server_side,
                rp,
                false);


    /* declare a long index variable for iterating over the array */
    hp->helpers.array_h.index =
        declare_short_local_var (
            NAMETABLE_add_derived_name (name, "%si_"),
            ulong_int_typep,
            both_sides,
            rp,
            false);

    /* declare a pointer variable to denote elements in loops ...*/
    hp->helpers.array_h.element_ptr_exp =
        declare_short_local_var (
            NAMETABLE_add_derived_name (name, "%sepe_"),
            pnp,
            both_sides,
            rp,
            false);

    /* declare a counter variable to bound loops ...*/
    hp->helpers.array_h.count_var =
        declare_short_local_var (
            NAMETABLE_add_derived_name (name, "%scv_"),
            ushort_int_typep,
            both_sides,
            rp,
            false);

    /* ... when allocating storage for this array cast it as a pointer to the element type */
    hp->helpers.array_h.alloc_type = pnp;

    /* create helper structure for array elements */
    hp->helpers.array_h.element =
        patch_node (
            (char *) NULL,      /* last_is_ref is appropriated by highest level array construct */
            (char *) NULL,      /* max_is_ref is appropriated by highest level array construct */
            NAMETABLE_add_derived_name (name, "%sel_"),
            NAMETABLE_add_derived_name (hp->helpers.array_h.element_ptr_exp, "(*%s)"),
            NAMETABLE_add_derived_name (hp->helpers.array_h.element_ptr_exp, "(*%s)"),
            *element_type_p,
            parameter_node,
            pp,
            rp
         );

    /* calculate size (in number of elements) of a major array slice */
    multiplier = 1;
    strcpy (index_string, ((*element_type_p)->kind == fixed_string_zero_k) ? "%s" : "%s[0]");
    for (indexp = indices->next; indexp; indexp = indexp->next) {
        multiplier = multiplier * cardinality (indexp->type->type_structure.subrange);
        strcat (index_string, "[0]");
        }

    /* record exps of form x[0][0]...[0] or *x for denoting the first element of an array */
    hp->helpers.array_h.cs_first_element_exp =
        NAMETABLE_add_derived_name (cs_mexp, index_string);
    hp->helpers.array_h.ss_first_element_exp =
        (array_kind == open_array_k && scalar_type (*element_type_p))
        ? NAMETABLE_add_derived_name (ss_mexp, "*%s")
        : NAMETABLE_add_derived_name (ss_mexp, index_string);

    /* find names for last_is and max_is terms depending
         on whether they are fields or parameters */
    if (parameter_node) {
        if (last_is_ref) {
            last_is_cs_exp = ((parameter_t *)last_is_ref)->helpers->cs_mexp;
            last_is_ss_exp = ((parameter_t *)last_is_ref)->helpers->ss_mexp;
            }
        if (max_is_ref) {
            max_is_cs_exp = ((parameter_t *)max_is_ref)->helpers->cs_mexp;
            max_is_ss_exp = ((parameter_t *)max_is_ref)->helpers->ss_mexp;
            }
        }
    else {
        /*
             Here, we have an array as a field of a record; its last_is and
             max_is terms are expressions denoting other fields in the record;
             we need a term of the form "parameter_name.last_is_name" and we
             have (in cs_mexp or ss_mexp) a nametable id for
             "parameter_name.array_field_name" and (in last_is_ref and
             max_is_ref) pointers to the fields containg the field names for
             the last_is and max_is fields.  Hence the following string
             manipulations.
        */

        if (last_is_ref) {
            /* get pointer to last_is field's name */
            NAMETABLE_id_to_string ( ((field_t *)last_is_ref)->name, &field_name_strp);

            /* get copy of cs_mexp string ... */
            NAMETABLE_id_to_string (cs_mexp, &mexp_strp);
            strcpy (work_str, mexp_strp);
            /* ... strip off array field name ... */
            for (i = strlen (work_str) - 1; work_str [i] != '.'; i--)
               ;
            work_str[i+1] = 0;
            /* ... append last_is field name ... */
            strcat (work_str, field_name_strp);
            /* ... and convert to to a nametable_id */
            last_is_cs_exp = NAMETABLE_add_id (work_str, false);

            /* get copy of ss_mexp string ... */
            NAMETABLE_id_to_string (ss_mexp, &mexp_strp);
            strcpy (work_str, mexp_strp);
            /* ... strip off array field name ... */
            for (i = strlen (work_str) - 1; work_str [i] != '.'; i--)
               ;
            work_str[i+1] = 0;
            /* ... append last_is field name ... */
            strcat (work_str, field_name_strp);
            /* ... and convert to to a nametable_id */
            last_is_ss_exp = NAMETABLE_add_id (work_str, false);
            }

        if (max_is_ref) {
            /* get pointer to max_is field's name */
            NAMETABLE_id_to_string ( ((field_t *)max_is_ref)->name, &field_name_strp);

            /* get copy of cs_mexp string ... */
            NAMETABLE_id_to_string (cs_mexp, &mexp_strp);
            strcpy (work_str, mexp_strp);
            /* ... strip off array field name ... */
            for (i = strlen (work_str) - 1; work_str [i] != '.'; i--)
               ;
            work_str[i+1] = 0;
            /* ... append max_is field name ... */
            strcat (work_str, field_name_strp);
            /* ... and convert to to a nametable_id */
            max_is_cs_exp = NAMETABLE_add_id (work_str, false);

            /* get copy of ss_mexp string ... */
            NAMETABLE_id_to_string (ss_mexp, &mexp_strp);
            strcpy (work_str, mexp_strp);
            /* ... strip off array field name ... */
            for (i = strlen (work_str) - 1; work_str [i] != '.'; i--)
               ;
            work_str[i+1] = 0;
            /* ... append max_is field name ... */
            strcat (work_str, field_name_strp);
            /* ... and convert to to a nametable_id */
            max_is_ss_exp = NAMETABLE_add_id (work_str, false);
            }

        }


    /* cook up an exp (possibly in terms of a last_is exp)
         for the number of elements in the array */
    if (last_is_ref) {
        hp->helpers.array_h.fixed_size = false;
        cs_limit_id = NAMETABLE_add_derived_name2 (
            last_is_cs_exp,
            constant_to_id (indices->type->type_structure.subrange.lower_bound),
            "(%s-%s+1)"
            );
        ss_limit_id = NAMETABLE_add_derived_name2 (
            last_is_ss_exp,
            constant_to_id (indices->type->type_structure.subrange.lower_bound),
            "(%s-%s+1)"
            );

        if (multiplier == 1) {
            hp->helpers.array_h.cs_limit_exp = cs_limit_id;
            hp->helpers.array_h.ss_limit_exp = ss_limit_id;
            }
        else {
            multiplier_id = NAMETABLE_add_numeric_exp (multiplier);
            hp->helpers.array_h.cs_limit_exp =
                NAMETABLE_add_derived_name2 (cs_limit_id, multiplier_id, "(%s*%s)");
            hp->helpers.array_h.ss_limit_exp =
                NAMETABLE_add_derived_name2 (ss_limit_id, multiplier_id, "(%s*%s)");
            }

        hp->helpers.array_h.check_bound = true;
        hp->helpers.array_h.bound_exp =
            (array_kind == open_array_k)
                ? NAMETABLE_add_id ("count", false)
                : NAMETABLE_add_numeric_exp (
                    multiplier * cardinality (indices->type->type_structure.subrange)
                    );

        
        }
    else {
        /* no last_is so use cardinality of first dimension */
        hp->helpers.array_h.cs_limit_exp =
        hp->helpers.array_h.ss_limit_exp =
            NAMETABLE_add_numeric_exp (
                multiplier * cardinality (indices->type->type_structure.subrange)
                );
        hp->helpers.array_h.fixed_size = true;

        hp->helpers.array_h.check_bound = false;
        }


    /* cook up an exp (possibly in terms of a max_is exp) for the number of elements
         to allocate for the array on the server side */
    if (max_is_ref) {
        /* # of elements to alloc is ("max_is" - base + 1) * multiplier */
        cs_alloc_exp_id = NAMETABLE_add_derived_name2 (
            max_is_cs_exp,
            constant_to_id (indices->type->type_structure.subrange.lower_bound),
            "(%s-%s+1)"
            );
        ss_alloc_exp_id = NAMETABLE_add_derived_name2 (
            max_is_ss_exp,
            constant_to_id (indices->type->type_structure.subrange.lower_bound),
            "(%s-%s+1)"
            );

        if (multiplier == 1) {
            hp->helpers.array_h.cs_alloc_exp = cs_alloc_exp_id;
            hp->helpers.array_h.ss_alloc_exp = ss_alloc_exp_id;
            }
        else {
            hp->helpers.array_h.cs_alloc_exp =
                NAMETABLE_add_derived_name2 (cs_alloc_exp_id, multiplier_id, "(%s*%s)");
            hp->helpers.array_h.ss_alloc_exp =
                NAMETABLE_add_derived_name2 (ss_alloc_exp_id, multiplier_id, "(%s*%s)");
            }
        }
    else {
        /* no "max_is" so use the appropriate limit exp derived above */
        hp->helpers.array_h.cs_alloc_exp = hp->helpers.array_h.cs_limit_exp;
        hp->helpers.array_h.ss_alloc_exp = hp->helpers.array_h.ss_limit_exp;
        }

    return hp;
}


static
#ifdef __STDC__
helpers_t *patch_node (
    char     *last_is_ref,
    char     *max_is_ref,
    NAMETABLE_id_t name,
    NAMETABLE_id_t cs_mexp,
    NAMETABLE_id_t ss_mexp,
    type_t         *type,
    boolean        parameter_flag,
    parameter_t    *pp,
    routine_t      *rp)
#else
helpers_t *patch_node (last_is_ref, max_is_ref, name, cs_mexp, ss_mexp, type, parameter_flag, pp, rp)
char     *last_is_ref;
char     *max_is_ref;
NAMETABLE_id_t name;        /* prefix of helper variable names */
NAMETABLE_id_t cs_mexp;     /* client size masrhalling expression */
NAMETABLE_id_t ss_mexp;     /* server size marshalling expression */
type_t         *type;
boolean        parameter_flag; /* true => this is a parameter node; false => field node */
parameter_t    *pp;
routine_t      *rp;
#endif
{
    helpers_t      *hp;
    type_t         *index_type_p;
    array_index_t  *indices;
    NAMETABLE_id_t marshall_exp, marshall_name;
    NAMETABLE_id_t unmarshall_exp, unmarshall_name;
    variant_t      *vp;
    helpers_t      *vhp;

    /* add local un/marshalling helper variables bindings associated with
         this parameter */

    switch (type->kind) {

        case boolean_k:
        case byte_k:
        case character_k:
        case short_integer_k:
        case small_integer_k:
        case long_integer_k:
        case hyper_integer_k:
        case short_unsigned_k:
        case small_unsigned_k:
        case long_unsigned_k:
        case hyper_unsigned_k:
        case short_bitset_k:
        case small_bitset_k:
        case long_bitset_k:
        case short_real_k:
        case long_real_k:
        case short_subrange_k:
        case small_subrange_k:
        case long_subrange_k:

        case short_enumeration_k:
        case small_enumeration_k:
        case long_enumeration_k:
            return alloc_helper_node (cs_mexp, ss_mexp);

        case pointer_k:
            return
                patch_node (
                    last_is_ref,
                    max_is_ref,
                    name,
                    NAMETABLE_add_derived_name (cs_mexp, "(*%s)"),
                    ss_mexp,
                    type->type_structure.pointer.pointee,
                    parameter_flag,
                    pp,
                    rp);

        case user_marshalled_k:
            hp = alloc_helper_node (cs_mexp, ss_mexp);

            /* For un/marshalling purposes a user_marshalled type is of its
                 [transmit_as()] type.  When marshalling such a value the user's
                 to_xmit_rep routine is called to yield a pointer to the xmissible
                 rep value whereas when unmarshalling the xmit rep value is managed
                 by the inline stub code.  Hence there are two serparate helpers_t
                 structures created for a user marshalled type: one for emitting
                 code to marshall parameters of user marshalled type and one
                 for emitting code to unmarshall them.
            */

            /* first construct the marshalling helper structure assuming a pointer to
                 the xmissible value (as return by the the to_xmit_rep routine) */
            marshall_name =
                declare_short_local_var (
                    NAMETABLE_add_derived_name (name, "%sxmissible_p_"),
                    pointer_type(type->type_structure.user_marshalled.xmit_type),
                    both_sides,
                    rp,
                    false);
            marshall_exp =
                    NAMETABLE_add_derived_name (marshall_name, "(*%s)");


            hp ->helpers.user_marshalled_h.marshall_ptr_var = marshall_name;

            hp ->helpers.user_marshalled_h.marshall_helpers =
                patch_node (
                    (char *) NULL,
                    (char *) NULL,
                    marshall_name,
                    marshall_exp,
                    marshall_exp,
                    type->type_structure.user_marshalled.xmit_type,
                    parameter_flag,
                    pp,
                    rp);

            /* construct the unmarshalling helpers as function of the xmissible type */
            if (open_type(type->type_structure.user_marshalled.xmit_type)) {
                unmarshall_name =
                    declare_short_local_var (
                        NAMETABLE_add_derived_name (name, "%sxmissible_"),
                        pointer_type (type->type_structure.user_marshalled.xmit_type),
                        both_sides,
                        rp,
                        false);
                unmarshall_exp =
                    NAMETABLE_add_derived_name (unmarshall_name, "(*%s)");
                }
            else
                unmarshall_name =
                unmarshall_exp =
                    declare_short_local_var (
                        NAMETABLE_add_derived_name (name, "%sxmissible_"),
                        type->type_structure.user_marshalled.xmit_type,
                        both_sides,
                        rp,
                        false);

            hp ->helpers.user_marshalled_h.unmarshall_helpers =
                patch_node (
                    (char *) NULL, /* Used to be NAMETABLE_NIL_ID ??? */
                    (char *) NULL, /* Used to be NAMETABLE_NIL_ID ??? */
                    unmarshall_name,
                    unmarshall_exp,
                    unmarshall_exp,
                    type->type_structure.user_marshalled.xmit_type,
                    parameter_flag,
                    pp,
                    rp);

            return hp;

        case open_record_k:
        case record_k:
            hp = alloc_helper_node (cs_mexp, ss_mexp);

            hp->helpers.record_h.fields =
                patch_field_list (
                    name,
                    cs_mexp,
                    ss_mexp,
                    type->type_structure.record.fields,
                    pp,
                    rp);

            if ((vp = type->type_structure.record.variant) != NULL) {
                vhp = alloc_helper_node (cs_mexp, ss_mexp);
                vhp->helpers.variant_h.tag =
                    patch_node (
                        (char *) NULL,
                        (char *) NULL,
                        NAMETABLE_add_derived_name2 (name, vp->tag_id, "%s%s_"),
                        NAMETABLE_add_derived_name2 (cs_mexp, vp->tag_id, "%s.%s"),
                        NAMETABLE_add_derived_name2 (ss_mexp, vp->tag_id, "%s.%s"),
                        vp->tag_type,
                        parameter_flag,
                        pp,
                        rp);
                vhp->helpers.variant_h.components =
                    patch_components (
                                      (parameter_t *)NULL,
                                      (parameter_t *)NULL,
                                      NAMETABLE_add_derived_name2 (name, vp->label, "%s%s_"),
                                      NAMETABLE_add_derived_name2 (cs_mexp, vp->label, "%s.%s"),
                                      NAMETABLE_add_derived_name2 (ss_mexp, vp->label, "%s.%s"),
                                      vp->components,
                                      pp,
                                      rp);

                hp->helpers.record_h.variant = vhp;
                }
            else {
                /* variant records and open records are mutually exclusive:
                   this is the "not variant record" case */
                hp->helpers.record_h.variant = NULL;
                if (open_type(type)) {
                    type->kind = open_record_k;
    
                    hp->helpers.record_h.alloc_var = name;
                    hp->helpers.record_h.alloc_cast_type = pointer_type (type);
                    hp->helpers.record_h.alloc_size_type = type;
    
                    hp->helpers.record_h.alloc_delta_exp = open_delta_exp (type, hp);
                    hp->helpers.record_h.alloc_delta_type = open_base_type (type);
                    }
                }

        return hp;

        case fixed_array_k:
            return
                patch_array_node (
                    last_is_ref,
                    max_is_ref,
                    name,
                    cs_mexp,
                    ss_mexp,
                    type->type_structure.fixed_array.indices,
                    &type->type_structure.fixed_array.elements,
                    fixed_array_k,
                    parameter_flag,
                    pp,
                    rp);

        case open_array_k:
            /* create a temporary index node to hold the place of the open index in the index list */

            index_type_p = gen_type_node (long_subrange_k);
            index_type_p->type_structure.subrange.lower_bound  = type->type_structure.open_array.lower_bound;

            indices = AST_index_node ((type_t *)NULL, false);
            indices->type = index_type_p;
            indices->next = type->type_structure.open_array.indices;

            hp =
                patch_array_node (
                    last_is_ref,
                    max_is_ref,
                    name,
                    cs_mexp,
                    ss_mexp,
                    indices,
                    &type->type_structure.open_array.elements,
                    open_array_k,
                    parameter_flag,
                    pp,
                    rp);

            type->type_structure.open_array.indices = indices->next;
            free ((char *)index_type_p);
            free ((char *)indices);
            return hp;


        case fixed_string_zero_k:
            hp = alloc_helper_node (cs_mexp, ss_mexp);
            hp->helpers.string_h.lenvar =
                declare_short_local_var (
                    NAMETABLE_add_derived_name (name, "%sstrlen_"),
                    ushort_int_typep,
                    both_sides,
                    rp,
                    false);
            return hp;

        default:
            unimp_type_exit (type->kind, "patch_node");
            break;

        }
    /*lint -unreachable */
}


static
#ifdef __STDC__
void declare_server_stub_standin (NAMETABLE_id_t name, type_t *type, routine_t *rp)
#else
void declare_server_stub_standin (name, type, rp)
NAMETABLE_id_t name;
type_t         *type;
routine_t      *rp;
#endif
{
    /* declare server side local variable for this parameter */

    switch (type->kind) {
        case pointer_k:
            declare_server_stub_standin (
                name,
                type->type_structure.pointer.pointee,
                rp);
            break;

        case open_array_k:
            declare_local_var (
                name,
                pointer_type (type->type_structure.open_array.elements),
                server_side,
                rp,
                NAMETABLE_NIL_ID,
                true);
            break;

        case open_record_k:
            declare_local_var (
                name,
                pointer_type (type),
                server_side,
                rp,
                NAMETABLE_NIL_ID,
                true);
            break;

        default:
            declare_local_var (
                name,
                type,
                server_side,
                rp,
                NAMETABLE_NIL_ID,
                false);
        }
}


static
#ifdef __STDC__
void patch_parameter_node (parameter_t *pp, routine_t *rp)
#else
void patch_parameter_node (pp, rp)
parameter_t *pp;
routine_t *rp;
#endif
{
    NAMETABLE_id_t ss_mexp;
    type_kind      kind;

    if (pp->patched)
        return;

    if (pp->last_is_ref != NULL)
        patch_parameter_node (pp->last_is_ref, rp);

    if (pp->max_is_ref != NULL)
        patch_parameter_node (pp->max_is_ref, rp);

    /* open types are denoted by pointers on server side (unless they are
         types that C treats as pointers anyway i.e. arrays and strings or
         unless they are user marshalled in which case they are denoted by
         pointers) */
    ss_mexp = pp->name;
    if (open_type(pp->type)) {
        if (pp->type->kind == pointer_k)
            kind = pp->type->type_structure.pointer.pointee->kind;
        else
            kind = pp->type->kind;
        switch (kind) {
            case open_array_k:
            case open_string_zero_k:
            case user_marshalled_k:
                break;
            default:
                ss_mexp = NAMETABLE_add_derived_name (ss_mexp, "(*%s)");
            }
        }

    /* derive and declare helper variables for parameter */
    pp->helpers =
        patch_node (
            (char *) pp->last_is_ref,
            (char *) pp->max_is_ref,
            pp->name,                 /* prefix for helper variable names */
            pp->name,                 /* client side marshalling exp */
            ss_mexp,                  /* server side marshalling exp */
            pp->type,
            true,                     /* this IS a parameter, NOT a field */
            pp,
            rp);

    pp->patched = true;
}


static
#ifdef __STDC__
void patch_routine_node (routine_t *rp, NAMETABLE_id_t name)
#else
void patch_routine_node (rp, name)
routine_t      *rp;   /* routine to be manipulated */
NAMETABLE_id_t name;  /* name of routine (used to name result parameter) */
#endif
{
    parameter_t    *pp;
    parameter_t    *prevp;
    parameter_t    sentinel;

    if (rp->backended)
        return;

    rp->backended = true;
    rp->any_ins = false;
    rp->any_outs = false;
    rp->any_opens = false;
    rp->comm_status_param = NULL;

    /* delete void result parameters and make non-void ones be outs and give them names */
    if (rp->result->type->kind == void_k)
        rp->result =  NULL;
    else {
        rp->result->in = false;
        rp->result->out = true;
        rp->result->name = name;
        }

    /* change each param name to avoid conflicts with derived names */
    for_params_and_result(pp, rp->parameters, rp->result)
        pp->name = NAMETABLE_add_derived_name (pp->name, "%s_");

    /* construct backend's list for un/marshalling parameters:
         - handles not in list
         - result "parameter" at end of list
    */
    sentinel.next_to_marshall = NULL;
    prevp = &sentinel;
    for_params_and_result (pp, rp->parameters, rp->result) {

        if (pp->type->kind == handle_k)
            continue;

        if ( (pp->type->kind == pointer_k)
             && (pp->type->type_structure.pointer.pointee->kind == handle_k)
             )
            continue;

        prevp->next_to_marshall = pp;
        prevp = pp;
        }
    prevp->next_to_marshall = NULL;
    rp->first_to_marshall = sentinel.next_to_marshall;

    /* pass over parameters, deriving variables and expression needed to
         marshall, unmarshall, and allocate storage in lient and stub routines
    */
    for_marshalling_list (pp, sentinel.next_to_marshall) {

        /* set flags for use in structuring stub code */
        if (pp->comm_status)  rp->comm_status_param = pp;
        if (pp->in)  rp->any_ins = true;
        if (pp->out) rp->any_outs = true;
        if ((!rp->any_opens) && (open_type (pp->type))) rp->any_opens = true;

        /* patch up this parameter node (build its helpers tree) */
        patch_parameter_node (pp, rp);

        declare_server_stub_standin (pp->name, pp->type, rp);
        }

    /* toss in decl of result parameter on client side also */
    if (rp->result)
        declare_local_var (
            rp->result->name,
            rp->result->type,
            client_side,
            rp,
            NAMETABLE_NIL_ID,
            false);

}


static
#ifdef __STDC__
long CSPELL_bound_calculation (FILE *fid, boolean last_is_ref, side_t side, type_t *tp, helpers_t *hp);
#else
long CSPELL_bound_calculation ();
#endif


static
#ifdef __STDC__
long CSPELL_fields_bound_calc (FILE *fid, side_t side, field_t *fp, helpers_t *fhp)
#else
long CSPELL_fields_bound_calc (fid, side, fp, fhp)
FILE        *fid;
side_t      side;
field_t     *fp;
helpers_t   *fhp;
#endif
{
    long    breakage = 0;
    boolean first = true;
    long    gap;
    long    prior_alignment=0;
    long    size = 0;

    for (; fp; fp = fp->next_field) {

        if (first)
            first = false;
        else {
            gap = required_alignment (fp->type, fp->last_is_ref != NULL) - prior_alignment;
            if (gap > 0)
                breakage += gap;
            }

        if (fp->next_field)
            prior_alignment = resulting_alignment (fp->type, (fp->last_is_ref != NULL));

        size +=
            CSPELL_bound_calculation (
                fid,
                (boolean) (fp->last_is_ref != NULL),
                side,
                fp->type,
                fhp);
        fhp = fhp->next;
        }

    return size + breakage;
}


static
#ifdef __STDC__
long CSPELL_bound_calculation (FILE *fid, boolean last_is_ref, side_t side, type_t *tp, helpers_t *hp)
#else
long CSPELL_bound_calculation (fid, last_is_ref, side, tp, hp)
FILE        *fid;
boolean     last_is_ref;
side_t      side;
type_t      *tp;
helpers_t   *hp;
#endif
{
    /*
         returns the size of the fixed size portion of an object of
         type tp->kind and emit code to increment the stub local
         variable 'bound' by the runtime size of dynamically sized
         portion
    */

    long           element_alignment;
    long           element_size;
    type_t         *element_type;
    long           gap;
    long           inter_element_breakage;
    long           prior_alignment;
    long           size;

    if (tp == NULL)
        return 0;

    if (size = fixed_size_type (tp, last_is_ref))
        return size;

    /* size == 0, i.e. the thing is not of fixed size at compile time */
    switch (tp->kind) {

        case pointer_k:
            return CSPELL_bound_calculation (
                fid,
                last_is_ref,
                side,
                tp->type_structure.pointer.pointee,
                hp);

        case  user_marshalled_k:
            return CSPELL_bound_calculation (
                fid,
                false,
                side,
                tp->type_structure.user_marshalled.xmit_type,
                hp->helpers.user_marshalled_h.marshall_helpers);

        case record_k:
            /* if here the record is not fixed size which implies
               (thanks to NIDL's semantic rules) that there is no variant
               part */
            return CSPELL_fields_bound_calc (
                fid,
                side,
                tp->type_structure.record.fields,
                hp->helpers.record_h.fields);

        case open_record_k:
            return CSPELL_fields_bound_calc (fid,
                side,
                tp->type_structure.record.fields,
                hp->helpers.record_h.fields);

        case open_array_k:
            element_type = tp->type_structure.open_array.elements;
            size = 4; /* account for the allocation and the count shorts in the rep */
            prior_alignment = 2;
            goto spell_array_bound_calculation;

        case fixed_array_k:
            element_type = tp->type_structure.fixed_array.elements;
            if (last_is_ref) {
                size = 2; /* account for the count short in the rep */
                prior_alignment = 2;
                }
            else {
                size = 0;
                prior_alignment = 1;
                }

        spell_array_bound_calculation:

            /* calculate max gap before first element and between_elements*/
            element_alignment = required_alignment (element_type, false);
            gap = element_alignment - prior_alignment;
            if (gap > 0)
                size += gap;
            inter_element_breakage =
                element_alignment -
                resulting_alignment (element_type, false);
            if (inter_element_breakage < 0)
                inter_element_breakage = 0;

            /* if element size is fixed then emit a runtime multiplication */
            if (element_size = fixed_size_type (element_type, false)) {
                element_size += inter_element_breakage;
                fprintf (fid, "bound += (");
                spell_name (fid,
                    (side == client_side)
                        ? hp->helpers.array_h.cs_limit_exp
                        : hp->helpers.array_h.ss_limit_exp);
                fprintf (fid, ") * %ld;\n", element_size);
                return size;
                }

            /* otherwise emit an iteration over array elements */
            spell_name (fid, hp->helpers.array_h.element_ptr_exp);
            fprintf (fid, "= ");
            if (element_type->kind != fixed_string_zero_k)
                fprintf (fid, "&");
            spell_name (fid,
                (side == client_side)
                    ? hp->helpers.array_h.cs_first_element_exp
                    : hp->helpers.array_h.ss_first_element_exp);
            fprintf (fid, ";\n");

            fprintf (fid, "for(");
            spell_name (fid, hp->helpers.array_h.index);
            fprintf (fid, "=");
            spell_name (fid,
                (side == client_side)
                    ? hp->helpers.array_h.cs_limit_exp
                    : hp->helpers.array_h.ss_limit_exp);
            fprintf (fid, "; ");
            spell_name (fid, hp->helpers.array_h.index);
            fprintf (fid, "; ");
            spell_name (fid, hp->helpers.array_h.index);
            fprintf (fid, "--){\n");

            element_size = inter_element_breakage +
                CSPELL_bound_calculation (
                    fid,
                    false,
                    side,
                    element_type,
                    hp->helpers.array_h.element);

            spell_name (fid, hp->helpers.array_h.element_ptr_exp);
            fprintf (fid, "++;\n");
            fprintf (fid, "}\n");

            if (element_size > 0) {
                fprintf (fid, "bound += %ld * (", element_size);
                spell_name (
                    fid,
                    (side == client_side)
                        ? hp->helpers.array_h.cs_limit_exp
                        : hp->helpers.array_h.ss_limit_exp);
                fprintf (fid, ");\n");
                }

            return size;

        case fixed_string_zero_k:
        case open_string_zero_k:
            fprintf    (fid, "bound += strlen((ndr_$char *) ");
            spell_name (
                fid,
                (side == client_side)
                    ? hp->cs_mexp
                    : hp->ss_mexp);
            fprintf    (fid, ");\n");
            return 3; /* 2 (short which holds the length) + 1 (\000 byte) */

        default:
            unimp_type_exit (tp->kind, "CSPELL_bound_calculation");
            break;

        }
    /*lint -unreachable */
}


static
#ifdef __STDC__
void CSPELL_call_to_xmit_routines (FILE *fid, param_kinds direction, side_t side, parameter_t *pp)
#else
void CSPELL_call_to_xmit_routines (fid, direction, side, pp)
FILE        *fid;
param_kinds direction;
side_t      side;
parameter_t *pp;
#endif
{
    type_t *tp;
    boolean first = true;


    for_marshalling_list (pp, pp) {
        /* skip parameters going in the wrong direction */
        if (direction == ins && !pp->in) continue;
        if (direction == outs && !pp->out) continue;

        /* skip parameters that aren't user_marshalled or pointers
           to user_marshalled types (i.e. user_marshalled outs) */
        if (((tp = pp->type)->kind != user_marshalled_k)
        && ((pp->type->kind != pointer_k)
            || ((tp = pp->type->type_structure.pointer.pointee)->kind != user_marshalled_k)))
            continue;

        if (first) {
            fprintf (fid, "\n/* convert user marshalled types */\n");
            first = false;
            }

        CSPELL_type_exp_simple (fid, tp);
        fprintf (fid, "_to_xmit_rep(");
        spell_name (
            fid,
            (side == client_side)
                ? pp->helpers->cs_mexp
                : pp->helpers->ss_mexp);
        fprintf (fid, ", &");
        spell_name (fid, pp->helpers->helpers.user_marshalled_h.marshall_ptr_var);
        fprintf (fid, ");\n");

        }
}


static
#ifdef __STDC__
void CSPELL_bound_calculations (FILE *fid, param_kinds direction, side_t side, parameter_t *pp)
#else
void CSPELL_bound_calculations (fid, direction, side, pp)
FILE        *fid;
param_kinds direction;
side_t      side;
parameter_t *pp;
#endif
{
    long    fixed_size_total = 0;
    boolean first = true;
    long    breakage = 0;
    long    gap;
    short    prior_alignment = 1;


    fprintf (fid, "bound=0;\n");

    for_marshalling_list (pp, pp) {

        if (direction == ins && !pp->in) continue;
        if (direction == outs && !pp->out) continue;

        if (first) {
            fprintf (fid, "\n/* bound calculations */\n");
            first = false;
            }
        else {
            gap = required_alignment (pp->type, pp->last_is_ref != NULL)
                - prior_alignment;
            if (gap > 0)
                breakage += gap;
            }
        if (pp->next_param)
            prior_alignment = resulting_alignment (pp->type, (pp->last_is_ref != NULL));

        fixed_size_total +=
            CSPELL_bound_calculation (
                fid,
                (boolean) (pp->last_is_ref != NULL),
                side,
                pp->type,
                pp->helpers);

        }

    if ((fixed_size_total + breakage) > 0)
        fprintf (fid, "bound += %ld;\n", fixed_size_total + breakage);
}


static
#ifdef __STDC__
void CSPELL_marshall (
    FILE       *fid,
    marshall_t un_marshall,
    type_t     *type,
    helpers_t  *hp,
    side_t     side,
    boolean    incr_mp,
    boolean    last_is_ref);
#else
void CSPELL_marshall ();
#endif


static
#ifdef __STDC__
void CSPELL_marshall_fields (FILE *fid, marshall_t un_marshall, field_t *fp, helpers_t *fhp, side_t side, boolean incr_mp)
#else
void CSPELL_marshall_fields (fid, un_marshall, fp, fhp, side, incr_mp)
FILE        *fid;
marshall_t  un_marshall;
field_t     *fp;
helpers_t   *fhp;
side_t      side;
boolean     incr_mp;
#endif
{
    for (; fp; fp = fp->next_field) {
        CSPELL_marshall (
            fid,
            un_marshall,
            fp->type,
            fhp,
            side,
            incr_mp ? true : fp->next_field != NULL,
            (fp->last_is_ref != NULL));
        fhp = fhp->next;
        }
}


static short mp_alignment = 1;

static
#ifdef __STDC__
void CSPELL_align_mp (FILE  *fid, short alignment)
#else
void CSPELL_align_mp (fid, alignment)
FILE  *fid;
short alignment;
#endif
{
    if (alignment > mp_alignment)
        fprintf (fid, "rpc_$align_ptr_relative (mp, dbp, %d);\n", alignment);
}


static
#ifdef __STDC__
void CSPELL_ele_ptr_init (FILE *fid, helpers_t *hp, type_t *element_type_p, side_t side)
#else
void CSPELL_ele_ptr_init (fid, hp, element_type_p, side)
FILE      *fid;
helpers_t *hp;
type_t    *element_type_p;
side_t    side;
#endif
{
    /* emit array element ptr initialization */
    spell_name (fid, hp->helpers.array_h.element_ptr_exp);
    fprintf (fid, " = ");
    if (element_type_p->kind != fixed_string_zero_k)
        fprintf (fid, "&");
    spell_name (fid,
        (side == client_side)
            ? hp->helpers.array_h.cs_first_element_exp
            : hp->helpers.array_h.ss_first_element_exp);
    fprintf (fid, ";\n");
}

static
#ifdef __STDC__
void CSPELL_marshall_array (
    FILE *fid,
    marshall_t  un_marshall,
    type_t *type,
    helpers_t *hp,
    side_t side,
    boolean incr_mp,
    boolean last_is_ref)
#else
void CSPELL_marshall_array (fid, un_marshall, type, hp, side, incr_mp, last_is_ref)
FILE        *fid;
marshall_t  un_marshall;
type_t      *type;
helpers_t   *hp;
side_t      side;
boolean     incr_mp;
boolean     last_is_ref;
#endif
{
    /*
        The marshalling strategy for arrays has two cases: (1) fixed
        size arrays without [LAST_IS()] attributes and (2) arrays with
        the [LAST_IS()] attribute including both fixed size and open
        arrays.  Fixed size arrays are simply represented by a sequence
        of reps of their fixed number of elements.  Varying sized arrays
        are marshalled by putting a short in the byte stream which gives
        the number element reps which follow.  Varying arrays are
        unmarshalled by extracting, and converting if necessary, this
        element count and then unmarshalling the specified number of
        elements from the byte stream.
    */

    short post_alignment, prior_alignment;
    type_t *element_type_p;
    boolean emitting_ifdef;

    if (!hp->helpers.array_h.fixed_size) {
        /* might be fixed_array_k but it has a [last_is()] so its rep includes an element count */
        CSPELL_align_mp (fid, 2);
        switch (un_marshall) {
            case marshall:
                /* emit code to put runtime element count into the byte stream */
                spell_name (fid, hp->helpers.array_h.count_var);
                fprintf (fid, " = ");
                spell_name (
                    fid,
                    (side == client_side)
                        ? hp->helpers.array_h.cs_limit_exp
                        : hp->helpers.array_h.ss_limit_exp
                    );
                fprintf (fid, ";\nrpc_$marshall_ushort_int(mp, ");
                spell_name (fid, hp->helpers.array_h.count_var);
                fprintf (fid, ");\n");
                break;

            case unmarshall:
                /* emit code to get element count into count var */
                fprintf (fid, "rpc_$unmarshall_ushort_int(mp, ");
                spell_name (fid, hp->helpers.array_h.count_var);
                fprintf (fid, ");\n");
                break;

            case convert:
                /* emit code to get element count into count var */
                fprintf (fid, "rpc_$convert_ushort_int (drep, rpc_$local_drep, mp, ");
                spell_name (fid, hp->helpers.array_h.count_var);
                fprintf (fid, ");\n");
                break;
            }
        fprintf (fid, "rpc_$advance_mp(mp, 2);\n");
        mp_alignment = 2;

        /* for open arrays emit code to signal an exception whenever the 
           actual number of elements exceeds the space available */
        if (hp->helpers.array_h.check_bound) {
            fprintf (fid, "if (");
            spell_name (fid, hp->helpers.array_h.count_var);
            fprintf (fid, ">");
            spell_name (fid, hp->helpers.array_h.bound_exp);
            fprintf (fid, ") SIGNAL(nca_status_$invalid_bound);\n");
            }

        }
    else
        /*  array is fixed size so there is no count to insert and extract
            from the byte stream; make the count_var be the (constant)
            limit exp so that the following loop emitting code can be
            oblivious */
        hp->helpers.array_h.count_var =
            (side == client_side)
                ? hp->helpers.array_h.cs_limit_exp
                : hp->helpers.array_h.ss_limit_exp;

    
    element_type_p = (type->kind == fixed_array_k) 
                     ? type->type_structure.fixed_array.elements
                     : type->type_structure.open_array.elements;

    /* emit special case logic to 'bcopy' arrays of scalars on machines
       whose idl_base.h define ALIGNED_SCALAR_ARRAYS */
    if (emitting_ifdef = (hp->helpers.array_h.scalar_elements && un_marshall != convert)) {
        fprintf (fid, "#ifdef ALIGNED_SCALAR_ARRAYS\n");

        prior_alignment = mp_alignment;
        CSPELL_align_mp (fid, required_alignment (element_type_p, false));

        if (  (side == server_side)
        && hp->helpers.array_h.use_ins_packet
        && (un_marshall == unmarshall)) {
            /* store the current value of mp in the variable that will
               be passed to the manager routine */
            spell_name (fid, hp->helpers.array_h.ss_arg_exp);
            fprintf (fid, " = ");
            CSPELL_cast_exp (fid, hp->helpers.array_h.alloc_type);
            fprintf (fid, "mp;\n");
            }
        else {
            CSPELL_ele_ptr_init (fid, hp, element_type_p, side);
            fprintf (fid, "rpc_$block_copy(");
            if (un_marshall == unmarshall)
                fprintf (fid, "(rpc_$byte_p_t)mp, ");
            fprintf (fid, "(rpc_$byte_p_t)");
            if (hp->helpers.array_h.use_ins_packet && (side == server_side))
                spell_name (fid, hp->helpers.array_h.ss_arg_exp);
            else
                spell_name (fid, hp->helpers.array_h.element_ptr_exp);
            if (un_marshall == marshall)
                fprintf (fid, ", (rpc_$byte_p_t)mp");
            fprintf (fid, ", (ndr_$ulong_int) (");
            spell_name (fid, hp->helpers.array_h.count_var);
            fprintf (fid, "*%ld));\n", fixed_size_type (element_type_p, false));
            }

        if (incr_mp) {
            fprintf (fid, "rpc_$advance_mp (mp, (");
            spell_name (fid, hp->helpers.array_h.count_var);
            fprintf (fid, "*%ld));\n", fixed_size_type (element_type_p, false));
            }

        mp_alignment = prior_alignment;
        fprintf (fid, "#else\n");
        }

    /* if attempting to use the ins packet for the server side rep of an array
       then we need to store the address of the server side surrogate when
       ALIGNED_SCALAR_ARRAYS is not defined */

    CSPELL_ele_ptr_init (fid, hp, element_type_p, side);
    if ((side == server_side) && hp->helpers.array_h.use_ins_packet) {
        spell_name (fid, hp->helpers.array_h.ss_arg_exp);
        fprintf (fid, " = ");
        spell_name (fid, hp->helpers.array_h.element_ptr_exp);
        fprintf (fid, ";\n");
        }

    /* emit loop to marshall each array element ... */
    /* ... emit loop header */
    fprintf (fid, "for (");
    spell_name (fid, hp->helpers.array_h.index);
    fprintf (fid, "=");
    spell_name (fid, hp->helpers.array_h.count_var);
    fprintf (fid, "; ");
    spell_name (fid, hp->helpers.array_h.index);
    fprintf (fid, "; ");
    spell_name (fid, hp->helpers.array_h.index);
    fprintf (fid, "--){\n");

    /* at the top of the loop alignment is minimum of alignment immediately
       before loop and alignment at end of loop */

    post_alignment = resulting_alignment (element_type_p, last_is_ref);
    if (post_alignment < mp_alignment)
        mp_alignment = post_alignment;
    
    
    /* ... emit loop body ... */
    CSPELL_marshall (
        fid,
        un_marshall,
        element_type_p,
        hp->helpers.array_h.element,
        side,
        true,
        false);

    /* ... with array element ptr incrementation */
    spell_name (fid, hp->helpers.array_h.element_ptr_exp);
    fprintf (fid, "++;\n");

    fprintf (fid, "}\n");

    if (emitting_ifdef)
        fprintf (fid, "#endif\n");

    mp_alignment = resulting_alignment (type, last_is_ref);
}


static
#ifdef __STDC__
void CSPELL_dealloc_param (FILE *fid, type_t *tp, side_t side, helpers_t *hp);
#else
void CSPELL_dealloc_param ();
#endif


static
#ifdef __STDC__
void CSPELL_user_marshall (FILE *fid, marshall_t un_marshall, type_t *type, helpers_t *hp, side_t side, boolean incr_mp)
#else
void CSPELL_user_marshall (fid, un_marshall, type, hp, side, incr_mp)
FILE        *fid;
marshall_t  un_marshall;
type_t      *type;
helpers_t   *hp;
side_t      side;
boolean     incr_mp;
#endif
{
    type_t *xmit_tp;

    xmit_tp = type->type_structure.user_marshalled.xmit_type;

    switch (un_marshall) {
        case marshall:
            /* emit code to marshall the xmit rep */
            CSPELL_marshall (
                fid,
                marshall,
                xmit_tp,
                hp->helpers.user_marshalled_h.marshall_helpers,
                side,
                incr_mp,
                false);

            /* emit call to <type>_free_xmit_rep () */
            CSPELL_type_exp_simple (fid, type);
            fprintf (fid, "_free_xmit_rep (");
            spell_name (fid, hp->helpers.user_marshalled_h.marshall_ptr_var);
            fprintf (fid, ");\n");
            break;

        case unmarshall:
        case convert:
            /* emit code to unmarshall the xmit rep */
            CSPELL_marshall (
                fid,
                un_marshall,
                xmit_tp,
                hp->helpers.user_marshalled_h.unmarshall_helpers,
                server_side,    /* server_side => allocation for open types which
                                                     is necessary for user unmarshalling even on client side */
                incr_mp,
                false);

            /* emit call to <type>_from_xmit_rep() */
            CSPELL_type_exp_simple (fid, type);
            fprintf (fid, "_from_xmit_rep (");
            if (!array_type (xmit_tp))
                fprintf (fid, "&");
            spell_name (
                fid,
                (side == client_side)
                    ? hp->helpers.user_marshalled_h.unmarshall_helpers->cs_mexp
                    : hp->helpers.user_marshalled_h.unmarshall_helpers->ss_mexp);
            fprintf (fid, ", &");
            spell_name (
                fid,
                (side == client_side)
                    ? hp->cs_mexp
                    : hp->ss_mexp);
            fprintf (fid, ");\n");
            CSPELL_dealloc_param (
                fid, xmit_tp, side,
                hp->helpers.user_marshalled_h.unmarshall_helpers);
            break;
        }
}


static
boolean marshalling_open_record = false;


static
#ifdef __STDC__
void CSPELL_marshall_open_array (FILE *fid, marshall_t  un_marshall, type_t *type, helpers_t *hp, side_t side, boolean incr_mp)
#else
void CSPELL_marshall_open_array (fid, un_marshall, type, hp, side, incr_mp)
FILE        *fid;
marshall_t  un_marshall;
type_t      *type;
helpers_t   *hp;
side_t      side;
boolean     incr_mp;
#endif
{
    /*
        The strategy for open arrays is: The marshaller emits a short integer
        into the byte stream which is the number of elements for which the
        server side stub should allocate storage.  The unmarshaller, if on
        the server side, extracts this number from the byte stream, converts
        it to native format if necessary, and allocates enough storage for
        that number of array elements.  If the unmarshaller is the client
        side it simply skips over this number.  Next the array is un/marshalled
        just as though it were not open.
    */

    CSPELL_align_mp (fid, 2);
    if (!marshalling_open_record) {
        switch (un_marshall) {
            case marshall:
                fprintf (fid, "count = ");
                spell_name (fid,
                    (side == client_side)
                    ?  hp->helpers.array_h.cs_alloc_exp
                    :  hp->helpers.array_h.ss_alloc_exp);
                fprintf (fid, ";\nrpc_$marshall_ushort_int (mp, count);\n");
                break;

            case unmarshall:
                if (side == server_side) {
                    /* emit code to get element count into local var 'count' */
                    fprintf (fid, "rpc_$unmarshall_ushort_int (mp, count);\n");

                    /* emit code to allocate array based on short in byte stream */
                    if (hp->helpers.array_h.use_ins_packet)
                        fprintf (fid, "#ifndef ALIGNED_SCALAR_ARRAYS\n");
                    spell_name (fid, hp->ss_mexp);
                    fprintf (fid, " = ");
                    CSPELL_cast_exp (fid, hp->helpers.array_h.alloc_type);
                    fprintf (fid, " rpc_$malloc (count * sizeof");
                    CSPELL_cast_exp (fid, type->type_structure.open_array.elements);
                    fprintf (fid, ");\n");
                    if (hp->helpers.array_h.use_ins_packet)
                        fprintf (fid, "#endif\n");
                    }
                break;

            case convert:
                if (side == server_side) {
                    /* emit code to get element count into local var 'count' */
                    fprintf (fid, "rpc_$convert_ushort_int(drep, rpc_$local_drep, mp, count);\n");

                    /* emit code to allocate array based on converted short in byte stream */
                    spell_name (fid, hp->ss_mexp);
                    fprintf (fid, "= ");
                    CSPELL_cast_exp (fid, hp->helpers.array_h.alloc_type);
                    fprintf (fid, " rpc_$malloc(count * sizeof");
                    CSPELL_cast_exp (fid, type->type_structure.open_array.elements);
                    fprintf (fid, ");\n");
                    }
                break;
            }

        if (un_marshall != marshall) /* unmarshall or convert */
            if (side == client_side) {
                /* insert init for "count" so the client side can check bounds
                   on output open arrays */
                fprintf (fid, "count = ");
                spell_name (fid, hp->helpers.array_h.cs_alloc_exp);
                fprintf (fid, ";\n");
                }
        fprintf (fid, "rpc_$advance_mp(mp, 2);\n");
        }
    mp_alignment = 2;

    CSPELL_marshall_array (fid, un_marshall, type, hp, side, incr_mp, true);
    }


#ifndef __STDC__
static void CSPELL_marshall_open_record (fid, un_marshall, type, hp, side, incr_mp)
FILE        *fid;
marshall_t  un_marshall;
type_t      *type;
helpers_t   *hp;
side_t      side;
boolean     incr_mp;
#else
static void CSPELL_marshall_open_record (FILE *fid, marshall_t un_marshall, type_t *type,
                                         helpers_t *hp, side_t side, boolean incr_mp)
#endif
{
    /*
        NDR defines an open record type whose rep in the byte stream is:
            max count of open array elements
            rep of fixed format part of record
            count (== N) of elements following in packet
            rep for N array elements

        The strategy for open records is: The marshaller emits a short integer
        into the byte stream which is the number of elements for which the
        server side stub should allocate storage in addition to the storage
        it allocates for the fixed fields of the record.  The unmarshaller,
        if on the server side, extracts this number from the byte stream,
        converts it to native format if necessary, and allocates enough storage
        for a record having that number of array elements.  If the unmarshaller
        is the client side it simply skips over this number.  Next the fields
        of the array are un/marshalled just as though they were in a fixed
        sized record.

        One detail is that the last thing un/marshalled in an open record
        is going to an open array and the allocation number for that open
        array has already been placed in the byte stream and should not be
        redundantly placed in front of the open array field.  Hence the global
        flag 'marshalling_open_record' is set to prevent
        CSPELL_marshall_open_array from emitting the allocation number code.
    */

    if (!marshalling_open_record) {
        CSPELL_align_mp (fid, 2);
        switch (un_marshall) {
            case marshall:
                fprintf (fid, "count = ");
                spell_name (fid, hp->helpers.record_h.alloc_delta_exp);
                fprintf (fid, ";\n");
                fprintf (fid, "rpc_$marshall_ushort_int (mp, count);\n");
                break;
    
            case convert:
                if (side == server_side) {
                    /* emit code to get element count into local var 'count' */
                    fprintf (fid, "rpc_$convert_ushort_int (drep, rpc_$local_drep, mp, count);\n");
                    goto emit_allocate_open_record;
                    }
                break;
    
            case unmarshall:
                if (side == server_side) {
                    /* emit code to get element count into local var 'count' */
                    fprintf (fid, "rpc_$unmarshall_ushort_int (mp, count);\n");
    
                    /* emit code to alloc storage for the open_record on the server side */
                emit_allocate_open_record:
                    spell_name (fid, hp->helpers.record_h.alloc_var);
                    fprintf (fid, "= ");
                    CSPELL_cast_exp (fid, hp->helpers.record_h.alloc_cast_type);
                    fprintf (fid, " rpc_$malloc(sizeof");
                    CSPELL_cast_exp (fid, hp->helpers.record_h.alloc_size_type);
                    fprintf (fid, "+((count-1)*(sizeof");
                    CSPELL_cast_exp (fid, hp->helpers.record_h.alloc_delta_type);
                    fprintf    (fid, ")));\n");
                    }
                break;
            }
    
        fprintf (fid, "rpc_$advance_mp(mp, 2);\n");
        mp_alignment = 2;
        }

    /* emit code to un/marshall the record's fields ... */
    marshalling_open_record = true;
    CSPELL_marshall_fields (
        fid,
        un_marshall,
        type->type_structure.record.fields,
        hp->helpers.record_h.fields,
        side,
        incr_mp);
    marshalling_open_record = false;
}


static
#ifdef __STDC__
void CSPELL_marshall_string_zero (FILE *fid, marshall_t un_marshall, type_t *type, helpers_t *hp,side_t side, type_kind kind, boolean incr_mp)
#else
void CSPELL_marshall_string_zero (fid, un_marshall, type, hp, side, kind, incr_mp)
FILE        *fid;
marshall_t  un_marshall;
type_t      *type;
helpers_t   *hp;
side_t      side;
type_kind   kind;
boolean     incr_mp;
#endif
{
    /* emit code to allocate max size for open strings on server side */
    if (side == server_side && kind == open_string_zero_k)
        switch (un_marshall) {
            case marshall:
                break;

            case unmarshall:
            case convert:
                spell_name (fid, hp->ss_mexp);
                fprintf    (fid, " = rpc_$malloc(");
                spell_name (fid, hp->helpers.string_h.alloc_exp);
                fprintf    (fid, ");\n");
                break;
            }

    /* gen code to move data for string_zero_k's */
    CSPELL_align_mp (fid, 2);
    fprintf (fid, "rpc_$");
    switch (un_marshall) {
        case unmarshall:
            fprintf (fid, "un");
        case marshall:
            fprintf (fid, "marshall_string(");
            break;
        case convert:
            fprintf (fid, "convert_string(drep, rpc_$local_drep, ");
            break;
        }
    spell_name (fid, hp->helpers.string_h.lenvar);
    fprintf    (fid, ", ");
    if (un_marshall == marshall)
        fprintf (
            fid,
            "%ld, ",
            cardinality (
                type->type_structure.fixed_string_zero.index->type->type_structure.subrange
                 )
            );            
    fprintf    (fid, "mp, ");
    spell_name (fid, (side == client_side) ? hp->cs_mexp : hp->ss_mexp);
    fprintf    (fid, ");\n");

    if (incr_mp) {
        fprintf (fid, "rpc_$advance_mp(mp, ");
        spell_name (fid, hp->helpers.string_h.lenvar);
        fprintf (fid, "+1);\n");
        }

    mp_alignment = 1;
}


static
#ifdef __STDC__
void CSPELL_marshall_variant (FILE *fid, marshall_t un_marshall, variant_t *vp, helpers_t *hp, side_t side, boolean incr_mp)
#else
void CSPELL_marshall_variant (fid, un_marshall, vp, hp, side, incr_mp)
FILE        *fid;
marshall_t  un_marshall;
variant_t   *vp;
helpers_t   *hp;
side_t      side;
boolean     incr_mp;
#endif
{
    helpers_t      *chp;
    component_t    *cp;
    NAMETABLE_id_t mexp;
    short           prior_alignment;
    tag_t          *tagp;
    type_kind      tag_kind;
    short           worst_case_alignment;
    boolean        not_defaulted_yet;

    /* marshall the tag value */
    CSPELL_marshall (
        fid,
        un_marshall,
        vp->tag_type,
        hp->helpers.variant_h.tag,
        side,
        true,
        false);

    /* emit a switch to marshall the appropriate variant */
    mexp = (side == client_side)
                     ? hp->helpers.variant_h.tag->cs_mexp
                     : hp->helpers.variant_h.tag->ss_mexp;
    fprintf (fid, "switch (");
    spell_name (fid, mexp);
    fprintf (fid, ") {\n");

    worst_case_alignment = rpc_$max_alignment;
    prior_alignment = mp_alignment;

    tag_kind = vp->tag_type->kind;
    not_defaulted_yet = true;

    chp = hp->helpers.variant_h.components;
    for (cp = vp->components; cp; cp = cp->next_component) {
        for (tagp = cp->tags; tagp; tagp = tagp->next_tag) {
            fprintf (fid, "case ");
            CSPELL_constant_val (fid, &(tagp->tag_value));
            fprintf (fid, ":\n"); 
            if (tag_kind == boolean_k && tagp->tag_value.value.boolean_value) {
                not_defaulted_yet = false;
                fprintf (fid, "default:\n");
                }
            }

        mp_alignment = prior_alignment;
        CSPELL_marshall_fields (
            fid,
            un_marshall,
            cp->fields,
            chp->helpers.component_h.fields,
            side,
            incr_mp);

        fprintf (fid, "break;\n");

        chp = chp->next;
        if (mp_alignment < worst_case_alignment)
            worst_case_alignment = mp_alignment;
        }

    if (not_defaulted_yet)
        fprintf (fid, "default: SIGNAL(nca_status_$invalid_tag);\n");

    fprintf (fid, "}\n");

    mp_alignment = worst_case_alignment;
}


static
#ifdef __STDC__
void CSPELL_marshall (
    FILE       *fid,
    marshall_t un_marshall,
    type_t     *type,
    helpers_t  *hp,
    side_t     side,
    boolean    incr_mp,
    boolean    last_is_ref)
#else
void CSPELL_marshall (fid, un_marshall, type, hp, side, incr_mp, last_is_ref)
FILE        *fid;
marshall_t  un_marshall;
type_t      *type;
helpers_t   *hp;
side_t      side;
boolean     incr_mp;
boolean     last_is_ref;
#endif
{
    NAMETABLE_id_t mexp;
    short          alignment;
    variant_t      *vp;

    /* marshall parameter */
    if (scalar_type(type)) {

        /* assure that the mp is aligned as required, no last_is_ref for scalars */
        CSPELL_align_mp (fid, (alignment = required_alignment (type, false)));
        mp_alignment = alignment;

        mexp = (side == client_side)
                         ? hp->cs_mexp
                         : hp->ss_mexp;

        fprintf (fid, "rpc_$");
            switch (un_marshall) {
                case unmarshall:
                    fprintf (fid, "un");
                case marshall:
                    fprintf (fid, "marshall_");
                    break;
                case convert:
                    fprintf (fid, "convert_");
                    break;
                }
        CSPELL_scalar_type_suffix (fid, type);
        fprintf (fid, "(");
        if (un_marshall == convert)
            fprintf (fid, "drep, rpc_$local_drep, ");
        fprintf (fid, "mp, ");
        spell_name (fid, mexp);
        fprintf (fid, ");\n");

        if (incr_mp)
            fprintf (fid, "rpc_$advance_mp(mp, %ld);\n", fixed_size_type (type, false));
        }
    else
        /* not a scalar type */
        switch (type->kind) {
            case pointer_k:
                CSPELL_marshall (
                    fid,
                    un_marshall,
                    type->type_structure.pointer.pointee,
                    hp,
                    side,
                    incr_mp,
                    false);
                break;

            case user_marshalled_k:
                CSPELL_user_marshall (
                    fid,
                    un_marshall,
                    type,
                    hp,
                    side,
                    incr_mp);
                break;

            case record_k:
                /* marshall the fixed fields ... */
                vp = type->type_structure.record.variant;
                CSPELL_marshall_fields (
                    fid,
                    un_marshall,
                    type->type_structure.record.fields,
                    hp->helpers.record_h.fields,
                    side,
                    (vp == NULL) ? incr_mp : true);
                /* if there is a variant part marshall it (as a dicriminated union) */
                if ((vp =type->type_structure.record.variant) != NULL)
                    CSPELL_marshall_variant (
                        fid,
                        un_marshall,
                        vp,
                        hp->helpers.record_h.variant,
                        side,
                        incr_mp);
                        
                break;


            case open_record_k:
                CSPELL_marshall_open_record (fid, un_marshall, type, hp, side, incr_mp);
                break;

            case open_array_k:
                CSPELL_marshall_open_array (fid, un_marshall, type, hp, side, incr_mp);
                break;

            case fixed_array_k:
                CSPELL_marshall_array (
                   fid, un_marshall, type, hp, side, incr_mp, last_is_ref);
                break;

            case fixed_string_zero_k:
            case open_string_zero_k:
                CSPELL_marshall_string_zero (fid, un_marshall, type, hp, side, type->kind, incr_mp);
                mp_alignment = 1;
                break;

            default:
                unimp_type_exit (type->kind, "CSPELL_marshall");
                break;

            }
}

static
#ifdef __STDC__
void CSPELL_marshalling (FILE *fid, param_kinds direction, parameter_t *pp, side_t side)
#else
void CSPELL_marshalling (fid, direction, pp, side)
FILE            *fid;
param_kinds     direction;
parameter_t     *pp;
side_t          side;
#endif
{
    boolean first = true;

    mp_alignment = rpc_$max_alignment;

    for_marshalling_list (pp, pp) {

        if (direction == ins && !pp->in) continue;
        if (direction == outs && !pp->out) continue;
        
        if (first) {
            first = false;
            fprintf (fid, "\n/* marshalling */\n");
            }

        CSPELL_marshall (
            fid,
            marshall,
            pp->type,
            pp->helpers,
            side,
            true,
            (pp->last_is_ref != NULL)
            );
        }
}


static
#ifdef __STDC__
boolean more_in_this_direction (parameter_t *pp, param_kinds direction)
#else
boolean more_in_this_direction (pp, direction)
parameter_t *pp;
param_kinds direction;
#endif
{
    switch (direction) {
        case ins:
            for_marshalling_list (pp, pp->next_to_marshall)
                if (pp->in)
                    return true;
            return false;
        case outs:
            for_marshalling_list (pp, pp->next_to_marshall)
                if (pp->out)
                    return true;
            return false;
        }
    /*lint -unreachable */
}


static
#ifdef __STDC__
void CSPELL_unmarshalling (FILE *fid, param_kinds direction, parameter_t *pp, side_t side, boolean space_opt)
#else
void CSPELL_unmarshalling (fid, direction, pp, side, space_opt)
FILE            *fid;
param_kinds     direction;
parameter_t     *pp;
side_t          side;
boolean         space_opt; /* omit fast path for when drep's match */
#endif
{
    parameter_t *temp_pp;
    boolean first = true;


    if (!space_opt) {
        fprintf (fid, "if (rpc_$equal_drep (drep, rpc_$local_drep)) {\n");
        mp_alignment = rpc_$max_alignment;
        for_marshalling_list (temp_pp, pp) {
            if (direction == ins && !temp_pp->in) continue;
            if (direction == outs && !temp_pp->out) continue;
            
            if (first) {
                first = false;
                fprintf (fid, "\n/* unmarshalling */\n");
                }

            CSPELL_marshall (
                fid,
                unmarshall,
                temp_pp->type,
                temp_pp->helpers,
                side,
                more_in_this_direction (temp_pp, direction),
                (temp_pp->last_is_ref != NULL)
                );
            }
        fprintf (fid, "} else {\n");
    }

    mp_alignment = rpc_$max_alignment;
    for_marshalling_list (temp_pp, pp) {
        if (direction == ins && !temp_pp->in) continue;
        if (direction == outs && !temp_pp->out) continue;

        if (first) {
            first = false;
            fprintf (fid, "\n/* unmarshalling */\n");
            }

        CSPELL_marshall (
            fid,
            convert,
            temp_pp->type,
            temp_pp->helpers,
            side,
            more_in_this_direction (temp_pp, direction),
            (temp_pp->last_is_ref != NULL)
            );
        }

    if (!space_opt)
        fprintf (fid, "}\n");
}

static
#ifdef __STDC__
void CSPELL_auto_unbinding (FILE *fid, NAMETABLE_id_t handle_type_name, NAMETABLE_id_t handle_var_name)
#else
void CSPELL_auto_unbinding (fid, handle_type_name, handle_var_name)
FILE *fid;
NAMETABLE_id_t handle_type_name;
NAMETABLE_id_t handle_var_name;
#endif
{
    fprintf (fid, "if (handle != NULL) ");
    spell_name (fid, handle_type_name);
    fprintf (fid, "_unbind(");
    spell_name (fid, handle_var_name);
    fprintf (fid, ", handle);\n");
}


static
#ifdef __STDC__
void CSPELL_auto_binding (FILE *fid, NAMETABLE_id_t handle_type_name, NAMETABLE_id_t handle_var_name)
#else
void CSPELL_auto_binding (fid, handle_type_name, handle_var_name)
FILE *fid;
NAMETABLE_id_t handle_type_name;
NAMETABLE_id_t handle_var_name;
#endif
{
    fprintf (fid, "handle = NULL;\nhandle = ");
    spell_name (fid, handle_type_name);
    fprintf (fid, "_bind(");
    spell_name (fid, handle_var_name);
    fprintf (fid, ");\n");
}


static
#ifdef __STDC__
void CSPELL_cs_cleanup_handler (
    FILE *fid,
    boolean any_ins,
    boolean auto_binding,
    parameter_t *comm_status_param,
    NAMETABLE_id_t handle_type_name,
    NAMETABLE_id_t handle_var_name)
#else
void CSPELL_cs_cleanup_handler (
    fid,
    any_ins,
    auto_binding,
    comm_status_param,
    handle_type_name,
    handle_var_name)
FILE *fid;
boolean any_ins;
boolean auto_binding;
parameter_t *comm_status_param;
NAMETABLE_id_t handle_type_name;
NAMETABLE_id_t handle_var_name;
#endif
{
    fprintf (fid, "cleanup_status = pfm_$cleanup (&cleanup_rec);\n");

    fprintf (fid, "if (cleanup_status.all != pfm_$cleanup_set) {\n");

    if (auto_binding) {
        fprintf (fid, "    ");
        CSPELL_auto_unbinding (fid, handle_type_name, handle_var_name);
        }

    if (any_ins) {
        fprintf (fid, "    if (free_ins) rpc_$free_pkt(ip);\n");
        }

    if (comm_status_param) {
        fprintf (fid, "    if (rpc_$is_comm_failure_status(cleanup_status)) {\n");
        fprintf (fid, "        *");
            spell_name (fid, comm_status_param->name);
            fprintf (fid, " = cleanup_status;\n");
        fprintf (fid, "        pfm_$enable();\n");
        fprintf (fid, "        return;\n");
        fprintf (fid, "        }\n");
        fprintf (fid, "    else\n");
        fprintf (fid, "        pfm_$signal (cleanup_status);\n");
        }
    else
        fprintf (fid, "    pfm_$signal (cleanup_status);\n");

    fprintf (fid, "    }\n\n");
}


static
#ifdef __STDC__
void CSPELL_client_stub_routine (FILE *fid, binding_t *rbp, NAMETABLE_id_t if_spec_name, NAMETABLE_id_t implicit_handle_var, type_t *implicit_handle_type, boolean space_opt)
#else
void CSPELL_client_stub_routine (
    fid, rbp, if_spec_name,
    implicit_handle_var, implicit_handle_type, space_opt)
FILE           *fid;
binding_t      *rbp;     /* routine bindings pointer */
NAMETABLE_id_t if_spec_name;
NAMETABLE_id_t implicit_handle_var;
type_t         *implicit_handle_type;
boolean        space_opt;
#endif
{
    routine_t      *rp;
    NAMETABLE_id_t handle_type_name;
    NAMETABLE_id_t handle_name;
    NAMETABLE_id_t rpc_handle_name;
    char           sar_opt_str[sizeof("0+rpc_$maybe+rpc_$idempotent+rpc_$brdcst")];
    char           *if_spec_str;
    boolean        handler_set;
    boolean        auto_binding;
    parameter_t    *first_param;
    type_t         *first_param_type;
    boolean        pointer_flag;

    rp = &rbp->binding->routine;

    pointerize_routine (rp);
    patch_routine_node (rp, rbp->name);

    CSPELL_csr_header (fid, rbp->name, rp->parameters, rp->result);

    /* figure out how to do binding for this routine */
    if (implicit_handle_var != NAMETABLE_NIL_ID)

        /* binding based on an implicit handle */
        if (implicit_handle_type->kind == handle_k) {
            /* 'manual' implicit binding */
            auto_binding = false;
            rpc_handle_name = implicit_handle_var;
            }
        else {
            /* 'auto' implicit binding */
            auto_binding = true;
            handle_type_name = implicit_handle_type->type_name;
            handle_name = implicit_handle_var;
            rpc_handle_name = NAMETABLE_add_id ("handle", false);
            }

    else {

        /* binding based on explicit first argument */
        first_param = rp->parameters;
        if (pointer_flag = (first_param->type->kind == pointer_k))
            first_param_type = first_param->type->type_structure.pointer.pointee;
        else
            first_param_type = first_param->type;

        if (first_param_type->kind == handle_k) {
            auto_binding = false;
            if (pointer_flag)
                rpc_handle_name = NAMETABLE_add_derived_name (first_param->name, "(*%s)");
            else
                rpc_handle_name = first_param->name;
            }
        else {
            auto_binding = true;
            handle_type_name = first_param_type->type_name;
            if (pointer_flag)
                handle_name = NAMETABLE_add_derived_name (first_param->name, "*%s");
            else
                handle_name = first_param->name;
            rpc_handle_name = NAMETABLE_add_id ("handle", false);
            }
        }

    /* emit body */

    fprintf (fid, "{\n");
    fprintf (fid, "\n/* rpc_$sar arguments */\n");
    fprintf (fid, "volatile rpc_$ppkt_t *ip;\n");
    fprintf (fid, "ndr_$ulong_int ilen;\n");
    fprintf (fid, "rpc_$ppkt_t *op;\n");
    fprintf (fid, "rpc_$ppkt_t *routs;\n");
    fprintf (fid, "ndr_$ulong_int olen;\n");
    fprintf (fid, "rpc_$drep_t drep;\n");
    fprintf (fid, "ndr_$boolean free_outs;\n");
    fprintf (fid, "status_$t st;\n");

    fprintf (fid, "\n/* other client side local variables */\n");
    fprintf (fid, "rpc_$ppkt_t ins;\n");
    fprintf (fid, "rpc_$ppkt_t outs;\n");

    if (auto_binding)
        fprintf (fid, "handle_t handle;\n");

    if (auto_binding || rp->any_ins) {
        fprintf (fid, "pfm_$cleanup_rec cleanup_rec;\n");
        fprintf (fid, "status_$t cleanup_status;\n");
        }

    if (rp->any_ins || rp->any_outs) {
        fprintf (fid, "ndr_$ushort_int data_offset;\n");
        fprintf (fid, "ndr_$ulong_int bound;\n");
        fprintf (fid, "rpc_$mp_t mp;\n");
        fprintf (fid, "rpc_$mp_t dbp;\n");
        fprintf (fid, "ndr_$ushort_int count;\n");
        }
    if (rp->any_ins)
        fprintf (fid, "volatile ndr_$boolean free_ins;\n");

    CSPELL_local_var_decls (fid, rp, client_side);

    if (handler_set = (auto_binding || rp->any_ins || rp->comm_status_param))
        CSPELL_cs_cleanup_handler (fid, 
            rp->any_ins,
            auto_binding,
            rp->comm_status_param,
            handle_type_name,
            handle_name);

    if (auto_binding)
        CSPELL_auto_binding (fid, handle_type_name, handle_name);

    if (rp->any_ins) {
        fprintf (fid, "\n/* marshalling init */\n");

        CSPELL_call_to_xmit_routines (fid, ins, client_side, rp->first_to_marshall);

        fprintf (fid, "data_offset=");
        spell_name (fid, rpc_handle_name);
        fprintf (fid, "->data_offset;\n");

        CSPELL_bound_calculations (fid, ins, client_side, rp->first_to_marshall);

        fprintf (fid, "\n/* buffer allocation */\n");
        fprintf (fid, "if(free_ins=(bound+data_offset>sizeof(rpc_$ppkt_t)))\n");
        fprintf (fid, "    ip=rpc_$alloc_pkt(bound);\n");
        fprintf (fid, "else \n");
        fprintf (fid, "    ip= &ins;\n");

        fprintf (fid, "rpc_$init_mp(mp, dbp, ip, data_offset);\n");

        CSPELL_marshalling (fid, ins, rp->first_to_marshall, client_side);

        }
    else
        fprintf (fid, "ip= &ins;\n");

    fprintf (fid, "\n/* runtime call */\n");
    if (rp->any_ins)
        fprintf (fid, "ilen=mp-dbp;\n");
    else
        fprintf (fid, "ilen=0;\n");

    fprintf (fid, "op= &outs;\n");

    fprintf (fid, "rpc_$sar(");
    spell_name (fid, rpc_handle_name);

    strcpy (sar_opt_str, "0");
    if (rp->broadcast)
        strcat (sar_opt_str, "+rpc_$brdcst");
    if (rp->idempotent)
        strcat (sar_opt_str, "+rpc_$idempotent");
    if (rp->maybe)
        strcat (sar_opt_str, "+rpc_$maybe");

    NAMETABLE_id_to_string (if_spec_name, &if_spec_str);

    fprintf (
        fid,
        ",\n (long)%s,\n &%s,\n %dL,\n ip,\n ilen,\n op,\n (long)sizeof(rpc_$ppkt_t),\n &routs,\n &olen,\n (rpc_$drep_t *)&drep,\n &free_outs,\n &st);\n",
        sar_opt_str, if_spec_str, rp->op_number);

    if (rp->any_outs) {
        fprintf (fid, "\n/* unmarshalling init */\n");
        if (!rp->any_ins) {
            fprintf (fid, "data_offset=");
            spell_name (fid, rpc_handle_name);
            fprintf (fid, "->data_offset;\n");
            }
        fprintf (fid, "rpc_$init_mp(mp, dbp, routs, data_offset);\n");

        CSPELL_unmarshalling (fid, outs, rp->first_to_marshall, client_side, space_opt);

        fprintf (fid, "\n/* buffer deallocation */\n");
        fprintf (fid, "if(free_outs) rpc_$free_pkt(routs);\n");
        }

    if (auto_binding)
        CSPELL_auto_unbinding (fid, handle_type_name, handle_name);

    if (rp->any_ins)
        fprintf (fid, "if(free_ins) rpc_$free_pkt(ip);\n");

    if (handler_set)
        fprintf (fid, "pfm_$rls_cleanup (&cleanup_rec, &cleanup_status);\n\n");

    if (rp->result) {
        fprintf (fid, "\n/* return */\n");
        fprintf (fid, "return(");
        spell_name (fid, rp->result->name);
        fprintf (fid, ");\n");
        }

    fprintf (fid, "}\n");
}


static void
#ifdef __STDC__
CSPELL_version_checking(FILE *fid)
#else
CSPELL_version_checking(fid)
FILE *fid;
#endif
{
    fprintf (fid, "#ifndef IDL_BASE_SUPPORTS_V1\n");
    fprintf (fid, "\"The version of idl_base.h is not compatible with this stub/switch code.\"\n");
    fprintf (fid, "#endif\n");

    fprintf (fid, "#ifndef RPC_IDL_SUPPORTS_V1\n");
    fprintf (fid, "\"The version of rpc.idl is not compatible with this stub/switch code.\"\n");
    fprintf (fid, "#endif\n");

    fprintf (fid, "#ifndef NCASTAT_IDL_SUPPORTS_V1\n");
    fprintf (fid, "\"The version of ncastat.idl is not compatible with this stub/switch code.\"\n");
    fprintf (fid, "#endif\n");
}


static
#ifdef __STDC__
void gen_client_stub (
    FILE *fid,
    NAMETABLE_id_t if_name,
    char leaf_name[],
    binding_t *rbp,
    NAMETABLE_id_t if_spec_name,
    NAMETABLE_id_t implicit_handle_var,
    type_t *implicit_handle_type,
     boolean space_opt)
#else
void gen_client_stub (
    fid, if_name, leaf_name, rbp, if_spec_name,
    implicit_handle_var, implicit_handle_type, space_opt
   )
FILE           *fid;
NAMETABLE_id_t if_name;                /* interface name */
char           leaf_name[];
binding_t      *rbp;                   /* routine bindings pointer */
NAMETABLE_id_t if_spec_name;
NAMETABLE_id_t implicit_handle_var;
type_t         *implicit_handle_type;
boolean        space_opt;
#endif
{
    boolean        first = true;
    binding_t      *temp_rbp;
    NAMETABLE_id_t type_name, var_name;

    CSPELL_std_include (fid, leaf_name, client_stub);
    CSPELL_version_checking (fid);
    fprintf (fid, "#include <ppfm.h>\n");

    /* emit instantiating declaration of implicit binding variable */
    if (implicit_handle_var != NAMETABLE_NIL_ID) {
        fprintf (fid, "globaldef ");
        CSPELL_type_exp_simple(fid, implicit_handle_type);
        fprintf(fid, " ");
        spell_name (fid, implicit_handle_var);
        fprintf (fid, ";\n");
        }

    /* emit client side stub routines */
    for (temp_rbp = rbp; temp_rbp; temp_rbp = temp_rbp->next)
        CSPELL_client_stub_routine (
            fid, temp_rbp, if_spec_name, implicit_handle_var, implicit_handle_type, space_opt);

    /* emit instantiating declaration of client epv and its initialization */
    var_name = NAMETABLE_add_derived_name (if_name, "%s$client_epv");
    type_name = NAMETABLE_add_derived_name (if_name, "%s$epv_t");
    fprintf (fid, "globaldef ");
    spell_name (fid, type_name);
    fprintf (fid, " ");
    spell_name (fid, var_name);
    fprintf (fid, " = {\n");
    for ( ; rbp; rbp = rbp->next) {
        if (first)
            first = false;
        else
            fprintf (fid, ",\n");
        spell_name (fid, NAMETABLE_add_derived_name (rbp->name, "%s_csr"));
        }
    fprintf (fid, "\n};\n");

}


static
#ifdef __STDC__
void CSPELL_init_open_outs (FILE *fid, parameter_t *pp)
#else
void CSPELL_init_open_outs (fid, pp)
FILE *fid;
parameter_t *pp;
#endif
{
    helpers_t *hp;
    type_t *tp;

    for_marshalling_list (pp, pp) {
        hp = pp->helpers;
        tp = pp->type;
        switch (tp->kind) {

            case open_record_k:
                spell_name (fid, hp->helpers.record_h.alloc_var);
                fprintf (fid, "= NULL;\n");
                break;

            case open_array_k:
                spell_name (fid, hp->ss_mexp);
                fprintf (fid, "= NULL;\n");
                break;

            default:
                break;
            }
        }
    fprintf (fid, "\n");
}


static
#ifdef __STDC__
void CSPELL_allocate_open_outs (FILE *fid, parameter_t *pp)
#else
void CSPELL_allocate_open_outs (fid, pp)
FILE *fid;
parameter_t *pp;
#endif
{
    helpers_t *hp;
    type_t *tp;

    for_marshalling_list (pp, pp)
        if (pp->out && !pp->in) {
            hp = pp->helpers;
            tp = pp->type;
            switch (tp->kind) {

                case open_record_k:
                    spell_name (fid, hp->helpers.record_h.alloc_var);
                    fprintf (fid, "= ");
                    CSPELL_cast_exp (fid, hp->helpers.record_h.alloc_cast_type);
                    fprintf (fid, " rpc_$malloc (sizeof");
                    CSPELL_cast_exp (fid, hp->helpers.record_h.alloc_size_type);
                    fprintf (fid, "+(");
                    spell_name (fid, hp->helpers.record_h.alloc_delta_exp);
                    fprintf (fid, "*sizeof");
                    CSPELL_cast_exp (fid, hp->helpers.record_h.alloc_delta_type);
                    fprintf    (fid, "));\n");
                    break;

                case open_array_k:
                    spell_name (fid, hp->ss_mexp);
                    fprintf (fid, " = ");
                    CSPELL_cast_exp (fid, hp->helpers.array_h.alloc_type);
                    fprintf (fid, " rpc_$malloc (");
                    spell_name (fid, hp->helpers.array_h.ss_alloc_exp);
                    fprintf (fid, " * sizeof");
                    CSPELL_cast_exp (fid, tp->type_structure.open_array.elements);
                    fprintf (fid, ");\n");
                    break;

                default:
                    break;
                }
            }
}


static
#ifdef __STDC__
void CSPELL_dealloc_param (FILE *fid, type_t *tp, side_t side, helpers_t *hp)
#else
void CSPELL_dealloc_param (fid, tp, side, hp)
FILE *fid;
type_t *tp;
side_t side;
helpers_t *hp;
#endif
{

    if (open_type (tp)) {

        switch (tp->kind) {

            case open_record_k:
                fprintf (fid, "if (");
                spell_name (fid, hp->helpers.record_h.alloc_var);
                fprintf (fid, " != NULL) rpc_$free((char *)");
                spell_name (fid, hp->helpers.record_h.alloc_var);
                fprintf (fid, ");\n");
                break;

            case open_string_zero_k:
            case open_array_k:
                fprintf (fid, "if (");
                spell_name (fid, hp->ss_mexp);
                fprintf (fid, " != NULL) rpc_$free((char *)");
                spell_name (fid, hp->ss_mexp);
                fprintf (fid, ");\n");
                break;

            case pointer_k:
                CSPELL_dealloc_param (fid, tp->type_structure.pointer.pointee, side, hp);
                break;

            case user_marshalled_k:
                /*
                 user marshalled types whose transmit_as types are open are
                 considered to be open by open_type(); however the storage
                 allocated for them is deallocated as part of unmarshalling
                 immediately after the _from_xmit_rep() call and therefore there is nothing
                 to do to deallocate user marshalled types xmit reps.
                */
                break;

            default:
                unimp_type_exit (tp->kind, "CSPELL_dealloc_param");
            }
        }
}



static void
#ifdef __STDC__
CSPELL_ss_dealloc_params (FILE *fid, parameter_t *pp)
#else
CSPELL_ss_dealloc_params (fid, pp)
FILE *fid;
parameter_t *pp;
#endif
{
    for_marshalling_list (pp, pp) {
        CSPELL_dealloc_param (fid, pp->type, server_side, pp->helpers);
        if (pp->type->kind == user_marshalled_k) {
            CSPELL_type_exp_simple (fid, pp->type);
            fprintf (fid, "_free (");
            spell_name (fid, pp->helpers->ss_mexp);
            fprintf (fid, ");\n");
            }
        }
}



static
#ifdef __STDC__
void CSPELL_server_call_argument (FILE *fid, NAMETABLE_id_t name, type_t *type, helpers_t *hp)
#else
void CSPELL_server_call_argument (fid, name, type, hp)
FILE           *fid;
NAMETABLE_id_t  name;
type_t         *type;
helpers_t      *hp;
#endif
{
    type_t *pointee;

    switch (type->kind) {

        case pointer_k:
            pointee = type->type_structure.pointer.pointee;

            if ((!open_type(pointee)) && (!array_type(pointee))
            || type->type_structure.pointer.pointee->kind == user_marshalled_k)
                fprintf (fid, "&");
            break;

        case open_array_k:
            fprintf (fid, "(");
            CSPELL_type_exp_simple (fid, hp->helpers.array_h.declared_element_type);
            fprintf (fid, " *)");
        case fixed_array_k:
            if (hp->helpers.array_h.use_ins_packet) {
                spell_name (fid, hp->helpers.array_h.ss_arg_exp);
                return;
                }
            break;

        default:
            break;
        }

    spell_name (fid, name);
}


static
#ifdef __STDC__
void CSPELL_server_call (
    FILE *fid,
    NAMETABLE_id_t name,
    parameter_t *pp,
    parameter_t *resultp,
    boolean f77_server,
    boolean mmmv)
#else
void CSPELL_server_call (fid, name, pp, resultp, f77_server, mmmv)
FILE *fid;
NAMETABLE_id_t name;
parameter_t *pp;
parameter_t *resultp;
boolean f77_server;
boolean mmmv; 
#endif
{
    boolean     first;

    fprintf (fid, "\n/* server call */\n");
    if (resultp && resultp->type->kind != void_k) {
        spell_name (fid, resultp->name);
        fprintf (fid, "=");
        }

    if (mmmv)
        fprintf (fid, "(*manager_epv->");

    spell_name (fid, name);
    /* append a "_" to server routine name if calling unix fortran code */
    if (f77_server)
        fprintf (fid, "_");

    if (mmmv)
        fprintf (fid, ")");

    fprintf(fid,"(");
    first = true;
    for (; pp; pp = pp->next_param) {
        if (first)
            first = false;
        else
            fprintf (fid, ", ");

        if (pp->type->kind == handle_k)
            fprintf (fid, "h");

        else if ((pp->type->kind == pointer_k)
                         && (pp->type->type_structure.pointer.pointee->kind == handle_k))
            fprintf (fid, "&h");

        else
            CSPELL_server_call_argument (fid, pp->name, pp->type, pp->helpers);
        }
    fprintf (fid, ");\n");
}


static
#ifdef __STDC__
void CSPELL_ss_cleanup_handler (
    FILE *fid,
    parameter_t *pp,
    boolean any_opens)
#else
void CSPELL_ss_cleanup_handler (
    fid,
    pp,
    any_opens)
FILE *fid;
parameter_t *pp;
boolean any_opens;
#endif
{
    helpers_t *hp;
    type_t *tp;

    fprintf (fid, "cleanup_status = pfm_$cleanup (&cleanup_rec);\n");

    fprintf (fid, "if (cleanup_status.all != pfm_$cleanup_set) {\n");

    if (any_opens)
        for_marshalling_list (pp, pp) {
            hp = pp->helpers;
            tp = pp->type;
            switch (tp->kind) {

                case open_record_k:
                    fprintf (fid, "    if (");
                    spell_name (fid, hp->helpers.record_h.alloc_var);
                    fprintf (fid, "!= NULL) rpc_$free(");
                    spell_name (fid, hp->helpers.record_h.alloc_var);
                    fprintf (fid, ");\n");
                    break;

                case open_array_k:
                    fprintf (fid, "    if (");
                    spell_name (fid, hp->ss_mexp);
                    fprintf (fid, "!= NULL) rpc_$free(");
                    spell_name (fid, hp->ss_mexp);
                    fprintf (fid, ");\n");
                    break;

                default:
                    break;
                }
            }

    fprintf (fid, "    pfm_$signal (cleanup_status);\n");

    fprintf (fid, "    }\n\n");
}


static
#ifdef __STDC__
void CSPELL_server_stub_routine (
    FILE *fid,
    binding_t *rbp,
    boolean f77_server,
    boolean space_opt,
    NAMETABLE_id_t epv_type_name)
#else
void CSPELL_server_stub_routine (fid, rbp, f77_server, space_opt, epv_type_name)
FILE      *fid;
binding_t *rbp; /* routine bindings pointer */
boolean   f77_server;
boolean   space_opt;
NAMETABLE_id_t epv_type_name;
#endif
{
    routine_t   *rp;
    boolean handler_set;

    rp = &rbp->binding->routine;

    pointerize_routine (rp);
    patch_routine_node (rp, rbp->name);

    fprintf (fid, "\nstatic void ");
    spell_name (fid, NAMETABLE_add_derived_name (rbp->name, "%s_ssr"));

    fprintf (fid, "\n#ifdef __STDC__\n");

        fprintf (fid, "(\n");
        fprintf (fid, " handle_t h,\n");
        fprintf (fid, " rpc_$ppkt_t *ins,\n");
        fprintf (fid, " ndr_$ulong_int ilen,\n");
        fprintf (fid, " rpc_$ppkt_t *outs,\n");
        fprintf (fid, " ndr_$ulong_int omax,\n");
        fprintf (fid, " rpc_$drep_t drep,\n");
        if (epv_type_name != NAMETABLE_NIL_ID) {
            fprintf (fid, " ");
            spell_name (fid, epv_type_name);
            fprintf (fid, " *manager_epv,\n");
            }
        fprintf (fid, " rpc_$ppkt_t **routs,\n");
        fprintf (fid, " ndr_$ulong_int *olen,\n");
        fprintf (fid, " ndr_$boolean *free_outs,\n");
        fprintf (fid, " status_$t *st\n)\n");

    fprintf (fid, "#else\n");

        fprintf (fid, "(\n h,\n ins,ilen,\n outs,omax,\n drep,\n");
        if (epv_type_name != NAMETABLE_NIL_ID)
            fprintf (fid, " manager_epv,\n");
        fprintf (fid, " routs,olen,\n free_outs,\n st)\n\n");
        fprintf (fid, "handle_t h;\n");
        fprintf (fid, "rpc_$ppkt_t *ins;\n");
        fprintf (fid, "ndr_$ulong_int ilen;\n");
        fprintf (fid, "rpc_$ppkt_t *outs;\n");
        fprintf (fid, "ndr_$ulong_int omax;\n");
        fprintf (fid, "rpc_$drep_t drep;\n");
        if (epv_type_name != NAMETABLE_NIL_ID) {
            spell_name (fid, epv_type_name);
            fprintf (fid, " *manager_epv;\n");
            }
        fprintf (fid, "rpc_$ppkt_t **routs;\n");
        fprintf (fid, "ndr_$ulong_int *olen;\n");
        fprintf (fid, "ndr_$boolean *free_outs;\n");
        fprintf (fid, "status_$t *st;\n");

    fprintf (fid, "#endif\n");

    fprintf (fid, "\n{\n");

    if (rp->any_ins || rp->any_outs) {
        fprintf (fid, "\n/* marshalling variables */\n");
        fprintf (fid, "ndr_$ushort_int data_offset;\n");
        fprintf (fid, "ndr_$ulong_int bound;\n");
        fprintf (fid, "rpc_$mp_t mp;\n");
        fprintf (fid, "rpc_$mp_t dbp;\n");
        fprintf (fid, "ndr_$ushort_int count;\n");
        }


    if (rp->any_opens) {
        fprintf (fid, "pfm_$cleanup_rec cleanup_rec;\n");
        fprintf (fid, "status_$t cleanup_status;\n");
        }

    CSPELL_local_var_decls (fid, rp, server_side);

    if (handler_set = rp->any_opens) {
        CSPELL_ss_cleanup_handler (fid, rp->first_to_marshall, rp->any_opens);
        CSPELL_init_open_outs (fid, rp->first_to_marshall);
        }

    if (rp->any_ins) {
        fprintf (fid, "\n/* unmarshalling init */\n");
        fprintf (fid, "data_offset=h->data_offset;\n");

        fprintf (fid, "rpc_$init_mp(mp, dbp, ins, data_offset);\n");

        CSPELL_unmarshalling (fid, ins, rp->first_to_marshall, server_side, space_opt);
        }

    if (rp->any_opens)
        CSPELL_allocate_open_outs (fid, rp->first_to_marshall);

    CSPELL_server_call (fid, rbp->name, rp->parameters, rp->result,
                        f77_server, (epv_type_name != NAMETABLE_NIL_ID));

    if (rp->any_outs) {

        CSPELL_call_to_xmit_routines (fid, outs, server_side, rp->first_to_marshall);

        if (!rp->any_ins)
            fprintf (fid, "data_offset=h->data_offset;\n");

        CSPELL_bound_calculations (fid, outs, server_side, rp->first_to_marshall);

        fprintf (fid, "\n/* buffer allocation */\n");
        fprintf (fid, "if(*free_outs=(bound>omax))\n");
        fprintf (fid, "    *routs=rpc_$alloc_pkt(bound);\n");
        fprintf (fid, "else\n");
        fprintf (fid, "    *routs=outs;\n");

        fprintf (fid, "rpc_$init_mp(mp, dbp, *routs, data_offset);\n");

        CSPELL_marshalling (fid, outs, rp->first_to_marshall, server_side);

        fprintf (fid, "\n*olen=mp-dbp;\n");
        }
    else {
        fprintf (fid, "\n/* buffer non-allocation */\n");
        fprintf (fid, "*free_outs=false;\n");
        fprintf (fid, "*routs=outs;\n");
        fprintf (fid, "*olen=0;\n");
        }

    CSPELL_ss_dealloc_params (fid, rp->first_to_marshall);

    if (handler_set)
        fprintf (fid, "pfm_$rls_cleanup (&cleanup_rec, &cleanup_status);\n\n");

    fprintf (fid, "\nst->all=status_$ok;\n");
    fprintf (fid, "}\n");
}


static
#ifdef __STDC__
void gen_server_stub (
    FILE *fid,
    NAMETABLE_id_t if_name,
    char leaf_name[],
    binding_t *rbp,
    boolean f77_server,
    boolean space_opt,
    boolean mmmv)
#else
void gen_server_stub (fid, if_name, leaf_name, rbp, f77_server, space_opt, mmmv)
FILE           *fid;
NAMETABLE_id_t if_name;
char           leaf_name[];
binding_t      *rbp; /* routine bindings pointer */
boolean        f77_server;
boolean        space_opt;
boolean        mmmv;
#endif
{
    boolean     first;
    binding_t   *temp_rbp;
    NAMETABLE_id_t var_name, type_name;
    NAMETABLE_id_t epv_name, epv_array_name;

    CSPELL_std_include (fid, leaf_name, server_stub);
    CSPELL_version_checking (fid);
    fprintf (fid, "#include <ppfm.h>\n");

    /* construct name for the epv type for managers implementing this interface */
    type_name = NAMETABLE_add_derived_name (if_name, "%s$epv_t");

    /* emit stub routine definitions */
    if (mmmv)
        for (temp_rbp = rbp; temp_rbp; temp_rbp = temp_rbp->next)
            CSPELL_server_stub_routine (fid, temp_rbp, f77_server, space_opt, type_name);
    else
        for (temp_rbp = rbp; temp_rbp; temp_rbp = temp_rbp->next)
            CSPELL_server_stub_routine (fid, temp_rbp, f77_server, space_opt, NAMETABLE_NIL_ID);

    /* for non-generic interfaces only: emit instantiating declaration of
       the manager entries epv and its initialization */
    if (!mmmv) {
        var_name = NAMETABLE_add_derived_name (if_name, "%s$manager_epv");
        fprintf (fid, "globaldef ");
        spell_name (fid, type_name);
        fprintf (fid, " ");
        spell_name (fid, var_name);
        fprintf (fid, " = {\n");
        first = true;
        for (temp_rbp = rbp; temp_rbp; temp_rbp = temp_rbp->next) {
    
            if (first)
                first = false;
            else
                fprintf (fid, ",\n");
            spell_name (fid, temp_rbp->name);
    
            /* append a "_" to server routine name if calling unix fortran code */
            if (f77_server)
                fprintf (fid, "_");
    
            }
        fprintf (fid, "\n};\n");
        }

    /* emit server epv declaration */
    epv_name = NAMETABLE_add_derived_name (if_name, "%s$server_epv");
    epv_array_name = NAMETABLE_add_derived_name (epv_name, "%sa");

    if (mmmv)
        fprintf (fid, "\nstatic rpc_$generic_server_stub_t ");
    else
        fprintf (fid, "\nstatic rpc_$server_stub_t ");
    spell_name (fid, epv_array_name);
    fprintf (fid, "[]={\n");
    first = true;
    for (temp_rbp = rbp; temp_rbp; temp_rbp = temp_rbp->next) {
        if (first)
            first = false;
        else
            fprintf (fid, ",\n");
        if (mmmv)
            fprintf (fid, "(rpc_$generic_server_stub_t)");
        else
            fprintf (fid, "(rpc_$server_stub_t)");
        spell_name (fid, NAMETABLE_add_derived_name (temp_rbp->name, "%s_ssr"));
        }
    fprintf (fid, "\n};\n");

    fprintf (fid, "globaldef ");
    if (mmmv)
        fprintf (fid, "rpc_$generic_epv_t ");
    else
        fprintf (fid, "rpc_$epv_t ");
    spell_name (fid, epv_name);
    if (mmmv)
        fprintf (fid, "=(rpc_$generic_epv_t)");
    else
        fprintf (fid, "=(rpc_$epv_t)");
    spell_name (fid, epv_array_name);
    fprintf (fid, ";\n");
}

#ifndef __STDC__
void
BACKEND_gen_c_stubs ( astp, cswfid, cstfid, sstfid, f77_client, f77_server, space_opt, mmmv)
binding_t *astp;
FILE      *cswfid;
FILE      *cstfid;
FILE      *sstfid;
boolean   f77_client;
boolean   f77_server;
boolean   space_opt;
boolean   mmmv;
#else
void BACKEND_gen_c_stubs  __PROTOTYPE((binding_t *astp, FILE *cswfid, FILE *cstfid, FILE *sstfid, boolean f77_client, boolean f77_server, boolean space_opt, boolean mmmv))
#endif
{
    interface_t    *ifp;
    binding_t      *routine_list;
    char           leaf_name [max_string_len];

    ifp = &astp->binding->interface;

    dename_interface (ifp);

    if (ifp->op_count) {
        routine_list = select_routine_bindings (ifp->exports);

        parse_path_name (ifp->source_file, (char *) NULL, leaf_name, (char *) NULL);

        gen_client_switch (
            cswfid,
            astp->name,
            leaf_name,
            routine_list,
            f77_client);
        gen_client_stub (
            cstfid,
            astp->name,
            leaf_name,
            routine_list,
            ifp->if_spec_name,
            ifp->implicit_handle_var,
            ifp->implicit_handle_type,
            space_opt);
        gen_server_stub (
            sstfid,
            astp->name,
            leaf_name,
            routine_list,
            f77_server,
            space_opt,
            mmmv);
    }
}
